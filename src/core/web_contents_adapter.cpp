/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "web_contents_adapter.h"

#include "browser_context_qt.h"
#include "content_browser_client_qt.h"
#include "javascript_dialog_manager_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

#include "base/values.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/page_zoom.h"
#include "content/public/common/renderer_preferences.h"
#include "ui/shell_dialogs/selected_file_info.h"

#include <QGuiApplication>
#include <QStringList>
#include <QStyleHints>
#include <QVariant>

static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;

static QVariant fromJSValue(const base::Value *result)
{
    QVariant ret;
    switch (result->GetType()) {
    case base::Value::TYPE_NULL:
        break;
    case base::Value::TYPE_BOOLEAN:
    {
        bool out;
        if (result->GetAsBoolean(&out))
            ret.setValue(out);
        break;
    }
    case base::Value::TYPE_INTEGER:
    {
        int out;
        if (result->GetAsInteger(&out))
            ret.setValue(out);
        break;
    }
    case base::Value::TYPE_DOUBLE:
    {
        double out;
        if (result->GetAsDouble(&out))
            ret.setValue(out);
        break;
    }
    case base::Value::TYPE_STRING:
    {
        base::string16 out;
        if (result->GetAsString(&out))
            ret.setValue(toQt(out));
        break;
    }
    case base::Value::TYPE_LIST:
    {
        const base::ListValue *out;
        if (result->GetAsList(&out)) {
            QVariantList list;
            list.reserve(out->GetSize());
            for (size_t i = 0; i < out->GetSize(); ++i) {
                const base::Value *outVal = 0;
                if (out->Get(i, &outVal) && outVal)
                    list.insert(i, fromJSValue(outVal));
            }
            ret.setValue(list);
        }
        break;
    }
    case base::Value::TYPE_DICTIONARY:
    {
        const base::DictionaryValue *out;
        if (result->GetAsDictionary(&out)) {
            QVariantMap map;
            base::DictionaryValue::Iterator it(*out);
            while (!it.IsAtEnd()) {
                map.insert(toQt(it.key()), fromJSValue(&it.value()));
                it.Advance();
            }
            ret.setValue(map);
        }
        break;
    }
    case base::Value::TYPE_BINARY:
    {
        const base::BinaryValue *out = static_cast<const base::BinaryValue*>(result);
        QByteArray data(out->GetBuffer(), out->GetSize());
        ret.setValue(data);
        break;
    }
    default:
        Q_UNREACHABLE();
        break;
    }
    return ret;
}

static void callbackOnEvaluateJS(JSCallbackBase *callback, const base::Value *result)
{
    callback->call(fromJSValue(result));
    delete callback;
}

static QStringList listRecursively(const QDir& dir) {
    QStringList ret;
    QFileInfoList infoList(dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot |QDir::Hidden));
    Q_FOREACH (const QFileInfo &fileInfo, infoList) {
        if (fileInfo.isDir()) {
            ret.append(fileInfo.absolutePath() + QStringLiteral("/.")); // Match chromium's behavior. See chrome/browser/file_select_helper.cc
            ret.append(listRecursively(QDir(fileInfo.absoluteFilePath())));
        } else
            ret.append(fileInfo.absoluteFilePath());
    }
    return ret;
}

class WebContentsAdapterPrivate {
public:
    WebContentsAdapterPrivate(WebContentsAdapterClient::RenderingMode renderingMode);
    scoped_refptr<WebEngineContext> engineContext;
    scoped_ptr<content::WebContents> webContents;
    scoped_ptr<WebContentsDelegateQt> webContentsDelegate;
    WebContentsAdapterClient *adapterClient;
};

WebContentsAdapterPrivate::WebContentsAdapterPrivate(WebContentsAdapterClient::RenderingMode renderingMode)
    // This has to be the first thing we create, and the last we destroy.
    : engineContext(WebEngineContext::currentOrCreate(renderingMode))
{
}

WebContentsAdapter::WebContentsAdapter(WebContentsAdapterClient::RenderingMode renderingMode, content::WebContents *webContents)
    : d_ptr(new WebContentsAdapterPrivate(renderingMode))
{
    Q_D(WebContentsAdapter);
    d->webContents.reset(webContents);
}

WebContentsAdapter::~WebContentsAdapter()
{
}

void WebContentsAdapter::initialize(WebContentsAdapterClient *adapterClient)
{
    Q_D(WebContentsAdapter);
    d->adapterClient = adapterClient;

    // Create our own if a WebContents wasn't provided at construction.
    if (!d->webContents) {
        content::BrowserContext* browserContext = ContentBrowserClientQt::Get()->browser_context();
        content::WebContents::CreateParams create_params(browserContext, NULL);
        create_params.routing_id = MSG_ROUTING_NONE;
        create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
        create_params.context = reinterpret_cast<gfx::NativeView>(adapterClient);
        d->webContents.reset(content::WebContents::Create(create_params));
    }

    content::RendererPreferences* rendererPrefs = d->webContents->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    const int qtCursorFlashTime = QGuiApplication::styleHints()->cursorFlashTime();
    rendererPrefs->caret_blink_interval = 0.5 * static_cast<double>(qtCursorFlashTime) / 1000;
    d->webContents->GetRenderViewHost()->SyncRendererPrefs();

    // Create and attach a WebContentsDelegateQt to the WebContents.
    d->webContentsDelegate.reset(new WebContentsDelegateQt(d->webContents.get(), adapterClient));

    // Let the WebContent's view know about the WebContentsAdapterClient.
    WebContentsViewQt* contentsView = static_cast<WebContentsViewQt*>(d->webContents->GetView());
    contentsView->initialize(adapterClient);
}

bool WebContentsAdapter::canGoBack() const
{
    Q_D(const WebContentsAdapter);
    return d->webContents->GetController().CanGoBack();
}

bool WebContentsAdapter::canGoForward() const
{
    Q_D(const WebContentsAdapter);
    return d->webContents->GetController().CanGoForward();
}

bool WebContentsAdapter::isLoading() const
{
    Q_D(const WebContentsAdapter);
    return d->webContents->IsLoading();
}

void WebContentsAdapter::stop()
{
    Q_D(WebContentsAdapter);
    content::NavigationController& controller = d->webContents->GetController();

    int index = controller.GetPendingEntryIndex();
    if (index != -1)
        controller.RemoveEntryAtIndex(index);

    d->webContents->GetView()->Focus();
}

void WebContentsAdapter::reload()
{
    Q_D(WebContentsAdapter);
    d->webContents->GetController().Reload(/*checkRepost = */false);
    d->webContents->GetView()->Focus();
}

void WebContentsAdapter::load(const QUrl &url)
{
    Q_D(WebContentsAdapter);
    content::NavigationController::LoadURLParams params(toGurl(url));
    params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
    d->webContents->GetController().LoadURLWithParams(params);
    d->webContents->GetView()->Focus();
}

QUrl WebContentsAdapter::activeUrl() const
{
    Q_D(const WebContentsAdapter);
    return toQt(d->webContents->GetVisibleURL());
}

QString WebContentsAdapter::pageTitle() const
{
    Q_D(const WebContentsAdapter);
    content::NavigationEntry* entry = d->webContents->GetController().GetVisibleEntry();
    return entry ? toQt(entry->GetTitle()) : QString();
}

void WebContentsAdapter::navigateToIndex(int offset)
{
    Q_D(WebContentsAdapter);
    d->webContents->GetController().GoToIndex(offset);
    d->webContents->GetView()->Focus();
}

void WebContentsAdapter::navigateToOffset(int offset)
{
    Q_D(WebContentsAdapter);
    d->webContents->GetController().GoToOffset(offset);
    d->webContents->GetView()->Focus();
}

int WebContentsAdapter::navigationEntryCount()
{
    Q_D(WebContentsAdapter);
    return d->webContents->GetController().GetEntryCount();
}

int WebContentsAdapter::currentNavigationEntryIndex()
{
    Q_D(WebContentsAdapter);
    return d->webContents->GetController().GetCurrentEntryIndex();
}

QUrl WebContentsAdapter::getNavigationEntryOriginalUrl(int index)
{
    Q_D(WebContentsAdapter);
    content::NavigationEntry *entry = d->webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetOriginalRequestURL()) : QUrl();
}

QUrl WebContentsAdapter::getNavigationEntryUrl(int index)
{
    Q_D(WebContentsAdapter);
    content::NavigationEntry *entry = d->webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetURL()) : QUrl();
}

QString WebContentsAdapter::getNavigationEntryTitle(int index)
{
    Q_D(WebContentsAdapter);
    content::NavigationEntry *entry = d->webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetTitle()) : QString();
}

void WebContentsAdapter::clearNavigationHistory()
{
    Q_D(WebContentsAdapter);
    if (d->webContents->GetController().CanPruneAllButVisible())
        d->webContents->GetController().PruneAllButVisible();
}

void WebContentsAdapter::setZoomFactor(qreal factor)
{
    Q_D(WebContentsAdapter);
    if (content::RenderViewHost *rvh = d->webContents->GetRenderViewHost())
        rvh->SetZoomLevel(content::ZoomFactorToZoomLevel(static_cast<double>(factor)));
}

qreal WebContentsAdapter::currentZoomFactor() const
{
    Q_D(const WebContentsAdapter);
    return static_cast<qreal>(content::ZoomLevelToZoomFactor(d->webContents->GetZoomLevel()));
}

void WebContentsAdapter::enableInspector(bool enable)
{
    ContentBrowserClientQt::Get()->enableInspector(enable);
}

void WebContentsAdapter::runJavaScript(const QString &javaScript, const QString &xPath, JSCallbackBase *func)
{
    Q_D(WebContentsAdapter);
    content::RenderViewHost *rvh = d->webContents->GetRenderViewHost();
    Q_ASSERT(rvh);
    if (!func)
        rvh->ExecuteJavascriptInWebFrame(toString16(xPath), toString16(javaScript));
    else {
        content::RenderViewHost::JavascriptResultCallback callback = base::Bind(&callbackOnEvaluateJS, func);
        rvh->ExecuteJavascriptInWebFrameCallbackResult(toString16(xPath), toString16(javaScript), callback);
    }
}

void WebContentsAdapter::dpiScaleChanged()
{
    Q_D(WebContentsAdapter);
    content::RenderWidgetHostImpl* impl = NULL;
    if (d->webContents->GetRenderViewHost())
        impl = content::RenderWidgetHostImpl::From(d->webContents->GetRenderViewHost());
    if (impl)
        impl->NotifyScreenInfoChanged();
}

void WebContentsAdapter::filesSelectedInChooser(const QStringList &fileList, WebContentsAdapterClient::FileChooserMode mode)
{
    Q_D(WebContentsAdapter);
    content::RenderViewHost *rvh = d->webContents->GetRenderViewHost();
    Q_ASSERT(rvh);
    QStringList files(fileList);
    if (mode == WebContentsAdapterClient::UploadFolder && !fileList.isEmpty()
            && QFileInfo(fileList.first()).isDir()) // Enumerate the directory
        files = listRecursively(QDir(fileList.first()));
    rvh->FilesSelectedInChooser(toVector<ui::SelectedFileInfo>(files), static_cast<content::FileChooserParams::Mode>(mode));
}
