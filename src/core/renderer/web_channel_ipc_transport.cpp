// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "renderer/web_channel_ipc_transport.h"

#include "content/public/renderer/render_frame.h"
#include "gin/arguments.h"
#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "v8/include/v8.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

namespace QtWebEngineCore {

class WebChannelTransport : public gin::Wrappable<WebChannelTransport>
{
public:
    static gin::WrapperInfo kWrapperInfo;
    static void Install(blink::WebLocalFrame *frame, uint worldId);
    static void Uninstall(blink::WebLocalFrame *frame, uint worldId);

private:
    WebChannelTransport() {}
    void NativeQtSendMessage(gin::Arguments *args);

    // gin::WrappableBase
    gin::ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate *isolate) override;
    mojo::AssociatedRemote<qtwebchannel::mojom::WebChannelTransportHost> m_remote;
    content::RenderFrame *m_renderFrame = nullptr;
};

gin::WrapperInfo WebChannelTransport::kWrapperInfo = { gin::kEmbedderNativeGin };

void WebChannelTransport::Install(blink::WebLocalFrame *frame, uint worldId)
{
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);
    v8::MicrotasksScope microtasks_scope(
        isolate, v8::MicrotasksScope::kDoNotRunMicrotasks);
    v8::Local<v8::Context> context;
    if (worldId == 0)
        context = frame->MainWorldScriptContext();
    else
        context = frame->IsolatedWorldScriptContext(worldId);
    if (context.IsEmpty())
        return;
    v8::Context::Scope contextScope(context);

    gin::Handle<WebChannelTransport> transport = gin::CreateHandle(isolate, new WebChannelTransport);
    if (transport.IsEmpty())
        return;

    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Value> qtObjectValue;
    v8::Local<v8::Object> qtObject;
    if (!global->Get(context, gin::StringToV8(isolate, "qt")).ToLocal(&qtObjectValue) || !qtObjectValue->IsObject()) {
        qtObject = v8::Object::New(isolate);
        global->Set(context, gin::StringToV8(isolate, "qt"), qtObject).Check();
    } else {
        qtObject = v8::Local<v8::Object>::Cast(qtObjectValue);
    }
    qtObject->Set(context, gin::StringToV8(isolate, "webChannelTransport"), transport.ToV8()).Check();
}

void WebChannelTransport::Uninstall(blink::WebLocalFrame *frame, uint worldId)
{
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context;
    if (worldId == 0)
        context = frame->MainWorldScriptContext();
    else
        context = frame->IsolatedWorldScriptContext(worldId);
    if (context.IsEmpty())
        return;
    v8::Context::Scope contextScope(context);

    v8::Local<v8::Object> global(context->Global());
    v8::Local<v8::Value> qtObjectValue;
    if (!global->Get(context, gin::StringToV8(isolate, "qt")).ToLocal(&qtObjectValue) || !qtObjectValue->IsObject())
        return;
    v8::Local<v8::Object> qtObject = v8::Local<v8::Object>::Cast(qtObjectValue);
    qtObject->Delete(context, gin::StringToV8(isolate, "webChannelTransport")).Check();
}

void WebChannelTransport::NativeQtSendMessage(gin::Arguments *args)
{
    blink::WebLocalFrame *frame = blink::WebLocalFrame::FrameForCurrentContext();
    if (!frame || !frame->View())
        return;

    content::RenderFrame *renderFrame = content::RenderFrame::FromWebFrame(frame);
    if (!renderFrame)
        return;

    v8::Local<v8::Value> jsonValue;
    if (!args->GetNext(&jsonValue)) {
        args->ThrowTypeError("Missing argument");
        return;
    }
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);

    if (!jsonValue->IsString()) {
        args->ThrowTypeError("Expected string");
        return;
    }
    v8::Local<v8::String> jsonString = v8::Local<v8::String>::Cast(jsonValue);
    std::vector<uint8_t> json(jsonString->Utf8Length(isolate), 0);
    jsonString->WriteUtf8(isolate, reinterpret_cast<char *>(json.data()), json.size(), nullptr,
                          v8::String::REPLACE_INVALID_UTF8);

    if (!m_remote) {
        renderFrame->GetRemoteAssociatedInterfaces()->GetInterface(&m_remote);
        m_renderFrame = renderFrame;
    }
    DCHECK(renderFrame == m_renderFrame);

    m_remote->DispatchWebChannelMessage(json);
}

gin::ObjectTemplateBuilder WebChannelTransport::GetObjectTemplateBuilder(v8::Isolate *isolate)
{
    return gin::Wrappable<WebChannelTransport>::GetObjectTemplateBuilder(isolate).SetMethod(
            "send", &WebChannelTransport::NativeQtSendMessage);
}

WebChannelIPCTransport::WebChannelIPCTransport(content::RenderFrame *renderFrame)
    : content::RenderFrameObserver(renderFrame)
    , m_worldId(0)
    , m_worldInitialized(false)
    , m_binding(this)
{
    renderFrame->GetAssociatedInterfaceRegistry()->AddInterface<qtwebchannel::mojom::WebChannelTransportRender>(
            base::BindRepeating(&WebChannelIPCTransport::BindReceiver, base::Unretained(this)));
}

void WebChannelIPCTransport::BindReceiver(
        mojo::PendingAssociatedReceiver<qtwebchannel::mojom::WebChannelTransportRender> receiver)
{
    m_binding.Bind(std::move(receiver));
}

void WebChannelIPCTransport::SetWorldId(uint32_t worldId)
{
    if (m_worldInitialized && m_worldId == worldId)
        return;

    if (m_worldInitialized && m_canUseContext)
        WebChannelTransport::Uninstall(render_frame()->GetWebFrame(), m_worldId);

    m_worldInitialized = true;
    m_worldId = worldId;

    if (m_canUseContext)
        WebChannelTransport::Install(render_frame()->GetWebFrame(), m_worldId);
}

void WebChannelIPCTransport::ResetWorldId()
{
    if (m_worldInitialized && m_canUseContext)
        WebChannelTransport::Uninstall(render_frame()->GetWebFrame(), m_worldId);

    m_worldInitialized = false;
    m_worldId = 0;
}

void WebChannelIPCTransport::DispatchWebChannelMessage(const std::vector<uint8_t> &json,
                                                       uint32_t worldId)
{
    DCHECK(m_worldId == worldId);

    if (!m_canUseContext)
        return;

    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context;
    if (worldId == 0)
        context = frame->MainWorldScriptContext();
    else
        context = frame->IsolatedWorldScriptContext(worldId);
    v8::Context::Scope contextScope(context);

    v8::Local<v8::Object> global(context->Global());
    v8::Local<v8::Value> qtObjectValue;
    if (!global->Get(context, gin::StringToV8(isolate, "qt")).ToLocal(&qtObjectValue) || !qtObjectValue->IsObject())
        return;
    v8::Local<v8::Object> qtObject = v8::Local<v8::Object>::Cast(qtObjectValue);
    v8::Local<v8::Value> webChannelObjectValue;
    if (!qtObject->Get(context, gin::StringToV8(isolate, "webChannelTransport")).ToLocal(&webChannelObjectValue)
            || !webChannelObjectValue->IsObject())
        return;
    v8::Local<v8::Object> webChannelObject = v8::Local<v8::Object>::Cast(webChannelObjectValue);
    v8::Local<v8::Value> callbackValue;
    if (!webChannelObject->Get(context, gin::StringToV8(isolate, "onmessage")).ToLocal(&callbackValue)
            || !callbackValue->IsFunction()) {
        LOG(WARNING) << "onmessage is not a callable property of qt.webChannelTransport. Some things might not work as "
                        "expected.";
        return;
    }

    v8::Local<v8::Object> messageObject(v8::Object::New(isolate));
    v8::Maybe<bool> wasSet = messageObject->DefineOwnProperty(
            context, v8::String::NewFromUtf8(isolate, "data").ToLocalChecked(),
            v8::String::NewFromUtf8(isolate, reinterpret_cast<const char *>(json.data()),
                                    v8::NewStringType::kNormal, json.size())
                    .ToLocalChecked(),
            v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
    DCHECK(!wasSet.IsNothing() && wasSet.FromJust());

    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(callbackValue);
    v8::Local<v8::Value> argv[] = { messageObject };
    frame->CallFunctionEvenIfScriptDisabled(callback, webChannelObject, 1, argv);
}

void WebChannelIPCTransport::DidCreateScriptContext(v8::Local<v8::Context> context, int32_t worldId)
{
    if (static_cast<uint>(worldId) == m_worldId && !m_canUseContext) {
        m_canUseContext = true;
        if (m_worldInitialized)
            WebChannelTransport::Install(render_frame()->GetWebFrame(), m_worldId);
    }
}

void WebChannelIPCTransport::WillReleaseScriptContext(v8::Local<v8::Context> context, int worldId)
{
    if (static_cast<uint>(worldId) == m_worldId)
        m_canUseContext = false;
}

void WebChannelIPCTransport::OnDestruct()
{
    delete this;
}

} // namespace QtWebEngineCore
