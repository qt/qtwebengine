/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/web_channel_ipc_transport.h"

#include "common/qt_messages.h"

#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#include <QJsonDocument>

namespace QtWebEngineCore {

static const char kWebChannelTransportExtensionName[] = "v8/WebChannelTransport";

static const char kWebChannelTransportApi[] =
        "if (typeof(qt) === 'undefined')" \
        "  qt = {};" \
        "if (typeof(qt.webChannelTransport) === 'undefined')" \
        "  qt.webChannelTransport = {};" \
        "qt.webChannelTransport.send = function(message) {" \
        "  native function NativeQtSendMessage();" \
        "  NativeQtSendMessage(message);" \
        "};";

class WebChannelTransportExtension : public v8::Extension {
public:
    static content::RenderView *GetRenderView();

    WebChannelTransportExtension() : v8::Extension(kWebChannelTransportExtensionName, kWebChannelTransportApi)
    {
    }

    virtual v8::Handle<v8::FunctionTemplate> GetNativeFunctionTemplate(v8::Isolate* isolate, v8::Handle<v8::String> name) Q_DECL_OVERRIDE;

    static void NativeQtSendMessage(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        content::RenderView *renderView = GetRenderView();
        if (!renderView || args.Length() != 1)
            return;
        v8::Handle<v8::Value> val = args[0];
        if (!val->IsString() && !val->IsStringObject())
            return;
        v8::String::Utf8Value utf8(val->ToString());

        QByteArray valueData(*utf8, utf8.length());
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(valueData, &error);
        if (error.error != QJsonParseError::NoError)
            qWarning("%s %d: Parsing error: %s",__FILE__, __LINE__, qPrintable(error.errorString()));
        int size = 0;
        const char *rawData = doc.rawData(&size);
        renderView->Send(new WebChannelIPCTransportHost_SendMessage(renderView->GetRoutingID(), std::vector<char>(rawData, rawData + size)));
    }
};

content::RenderView *WebChannelTransportExtension::GetRenderView()
{
    blink::WebLocalFrame *webframe = blink::WebLocalFrame::frameForCurrentContext();
    DCHECK(webframe) << "There should be an active frame since we just got a native function called.";
    if (!webframe)
        return 0;

    blink::WebView *webview = webframe->view();
    if (!webview)
        return 0;  // can happen during closing

    return content::RenderView::FromWebView(webview);
}

v8::Handle<v8::FunctionTemplate> WebChannelTransportExtension::GetNativeFunctionTemplate(v8::Isolate *isolate, v8::Handle<v8::String> name)
{
    if (name->Equals(v8::String::NewFromUtf8(isolate, "NativeQtSendMessage")))
        return v8::FunctionTemplate::New(isolate, NativeQtSendMessage);

    return v8::Handle<v8::FunctionTemplate>();
}

WebChannelIPCTransport::WebChannelIPCTransport(content::RenderView *renderView)
    : content::RenderViewObserver(renderView)
{
}

void WebChannelIPCTransport::dispatchWebChannelMessage(const std::vector<char> &binaryJSON)
{
    blink::WebView *webView = render_view()->GetWebView();
    if (!webView)
        return;

    QJsonDocument doc = QJsonDocument::fromRawData(binaryJSON.data(), binaryJSON.size(), QJsonDocument::BypassValidation);
    Q_ASSERT(doc.isObject());
    QByteArray json = doc.toJson(QJsonDocument::Compact);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    blink::WebFrame *frame = webView->mainFrame();
    v8::Handle<v8::Context> context = frame->mainWorldScriptContext();
    v8::Context::Scope contextScope(context);

    v8::Handle<v8::Object> global(context->Global());
    v8::Handle<v8::Value> qtObjectValue(global->Get(v8::String::NewFromUtf8(isolate, "qt")));
    if (!qtObjectValue->IsObject())
        return;
    v8::Handle<v8::Value> webChannelObjectValue(qtObjectValue->ToObject()->Get(v8::String::NewFromUtf8(isolate, "webChannelTransport")));
    if (!webChannelObjectValue->IsObject())
        return;
    v8::Handle<v8::Value> onmessageCallbackValue(webChannelObjectValue->ToObject()->Get(v8::String::NewFromUtf8(isolate, "onmessage")));
    if (!onmessageCallbackValue->IsFunction()) {
        qWarning("onmessage is not a callable property of qt.webChannelTransport. Some things might not work as expected.");
        return;
    }

    v8::Handle<v8::Object> messageObject(v8::Object::New(isolate));
    messageObject->ForceSet(v8::String::NewFromUtf8(isolate, "data")
                       , v8::String::NewFromUtf8(isolate, json.constData(), v8::String::kNormalString, json.size())
                       , v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));

    v8::Handle<v8::Function> callback = v8::Handle<v8::Function>::Cast(onmessageCallbackValue);
    const int argc = 1;
    v8::Handle<v8::Value> argv[argc];
    argv[0] = messageObject;
    frame->callFunctionEvenIfScriptDisabled(callback, webChannelObjectValue->ToObject(), argc, argv);
}

v8::Extension *WebChannelIPCTransport::getV8Extension()
{
    return new WebChannelTransportExtension;
}

bool WebChannelIPCTransport::OnMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(WebChannelIPCTransport, message)
        IPC_MESSAGE_HANDLER(WebChannelIPCTransport_Message, dispatchWebChannelMessage)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
}

} // namespace
