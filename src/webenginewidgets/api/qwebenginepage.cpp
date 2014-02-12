/*
    Copyright (C) 2012, 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008, 2009, 2012 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.
    Copyright (C) 2007 Apple Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qwebenginepage.h"
#include "qwebenginepage_p.h"

#include "javascript_dialog_controller.h"
#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"
#include "qwebengineview.h"
#include "qwebengineview_p.h"
#include "render_widget_host_view_qt_delegate_popup.h"
#include "render_widget_host_view_qt_delegate_webpage.h"
#include "web_contents_adapter.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include <QUrl>

QT_BEGIN_NAMESPACE

QWebEnginePagePrivate::QWebEnginePagePrivate()
    : QObjectPrivate(QObjectPrivateVersion)
    , adapter(new WebContentsAdapter(SoftwareRenderingMode))
    , history(new QWebEngineHistory(new QWebEngineHistoryPrivate(adapter.data())))
    , view(0)
    , m_isLoading(false)
{
    memset(actions, 0, sizeof(actions));
}

QWebEnginePagePrivate::~QWebEnginePagePrivate()
{
    // "Cancel" pending callbacks by calling them with an invalid value.
    // This guarantees that each callback is called exactly once.
    Q_FOREACH (QExplicitlySharedDataPointer<VariantCallback> callback, m_variantCallbacks)
        (*callback)(QVariant());
    m_variantCallbacks.clear();
    Q_FOREACH (QExplicitlySharedDataPointer<StringCallback> callback, m_stringCallbacks)
        (*callback)(QString());
    m_stringCallbacks.clear();

    delete history;
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client, RenderingMode mode)
{
    Q_UNUSED(mode);
    return new RenderWidgetHostViewQtDelegateWebPage(client);
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client, WebContentsAdapterClient::RenderingMode)
{
    Q_ASSERT(m_rwhvDelegate);
    return new RenderWidgetHostViewQtDelegatePopup(client, view);
}

void QWebEnginePagePrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->titleChanged(title);
}

void QWebEnginePagePrivate::urlChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    m_explicitUrl = QUrl();
    Q_EMIT q->urlChanged(url);
}

void QWebEnginePagePrivate::iconChanged(const QUrl &url)
{
    Q_UNUSED(url)
}

void QWebEnginePagePrivate::loadingStateChanged()
{
    Q_Q(QWebEnginePage);
    const bool wasLoading = m_isLoading;
    m_isLoading = adapter->isLoading();
    if (m_isLoading != wasLoading) {
        if (m_isLoading)
            Q_EMIT q->loadStarted();
    }
    updateNavigationActions();
}

void QWebEnginePagePrivate::loadProgressChanged(int progress)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->loadProgress(progress);
}

QRectF QWebEnginePagePrivate::viewportRect() const
{
    return view ? view->geometry() : QRectF();
}

qreal QWebEnginePagePrivate::dpiScale() const
{
    return 1.0;
}

void QWebEnginePagePrivate::loadFinished(bool success, int error_code, const QString &error_description)
{
    Q_Q(QWebEnginePage);
    Q_UNUSED(error_code);
    Q_UNUSED(error_description);
    m_isLoading = adapter->isLoading();
    if (success)
        m_explicitUrl = QUrl();
    Q_EMIT q->loadFinished(success);
}

void QWebEnginePagePrivate::focusContainer()
{
    if (view)
        view->setFocus();
}

void QWebEnginePagePrivate::adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, const QRect &initialGeometry)
{
    Q_Q(QWebEnginePage);
    QWebEnginePage *newPage = q->createWindow(disposition == WebContentsAdapterClient::NewPopupDisposition ? QWebEnginePage::WebModalDialog : QWebEnginePage::WebBrowserWindow);
    // Overwrite the new page's WebContents with ours.
    if (newPage) {
        newPage->d_func()->adapter = newWebContents;
        newWebContents->initialize(newPage->d_func());
        if (!initialGeometry.isEmpty())
            emit newPage->geometryChangeRequested(initialGeometry);
    }
}

void QWebEnginePagePrivate::close()
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->windowCloseRequested();
}

void QWebEnginePagePrivate::didRunJavaScript(quint64 requestId, const QVariant& result)
{
    if (QExplicitlySharedDataPointer<VariantCallback> callback = m_variantCallbacks.take(requestId))
        (*callback)(result);
}

void QWebEnginePagePrivate::didFetchDocumentMarkup(quint64 requestId, const QString& result)
{
    if (QExplicitlySharedDataPointer<StringCallback> callback = m_stringCallbacks.take(requestId))
        (*callback)(result);
}

void QWebEnginePagePrivate::didFetchDocumentInnerText(quint64 requestId, const QString& result)
{
    if (QExplicitlySharedDataPointer<StringCallback> callback = m_stringCallbacks.take(requestId))
        (*callback)(result);
}

void QWebEnginePagePrivate::updateAction(QWebEnginePage::WebAction action) const
{
#ifdef QT_NO_ACTION
    Q_UNUSED(action)
#else
    QAction *a = actions[action];
    if (!a)
        return;

    bool enabled = false;

    switch (action) {
    case QWebEnginePage::Back:
        enabled = adapter->canGoBack();
        break;
    case QWebEnginePage::Forward:
        enabled = adapter->canGoForward();
        break;
    case QWebEnginePage::Stop:
        enabled = adapter->isLoading();
        break;
    case QWebEnginePage::Reload:
    case QWebEnginePage::ReloadAndBypassCache:
        enabled = !adapter->isLoading();
        break;
    default:
        break;
    }

    a->setEnabled(enabled);
#endif // QT_NO_ACTION
}

void QWebEnginePagePrivate::updateNavigationActions()
{
    updateAction(QWebEnginePage::Back);
    updateAction(QWebEnginePage::Forward);
    updateAction(QWebEnginePage::Stop);
    updateAction(QWebEnginePage::Reload);
    updateAction(QWebEnginePage::ReloadAndBypassCache);
}

#ifndef QT_NO_ACTION
void QWebEnginePagePrivate::_q_webActionTriggered(bool checked)
{
    Q_Q(QWebEnginePage);
    QAction *a = qobject_cast<QAction *>(q->sender());
    if (!a)
        return;
    QWebEnginePage::WebAction action = static_cast<QWebEnginePage::WebAction>(a->data().toInt());
    q->triggerAction(action, checked);
}
#endif // QT_NO_ACTION

QWebEnginePage::QWebEnginePage(QObject* parent)
    : QObject(*new QWebEnginePagePrivate, parent)
{
    Q_D(QWebEnginePage);
    d->adapter->initialize(d);
}

QWebEnginePage::~QWebEnginePage()
{
    Q_D(QWebEnginePage);
    QWebEngineViewPrivate::bind(d->view, 0);
}

QWebEngineHistory *QWebEnginePage::history() const
{
    Q_D(const QWebEnginePage);
    return d->history;
}

void QWebEnginePage::setView(QWidget *view)
{
    QWebEngineViewPrivate::bind(qobject_cast<QWebEngineView*>(view), this);
}

QWidget *QWebEnginePage::view() const
{
    Q_D(const QWebEnginePage);
    return d->view;
}

#ifndef QT_NO_ACTION
QAction *QWebEnginePage::action(WebAction action) const
{
    Q_D(const QWebEnginePage);
    if (action == QWebEnginePage::NoWebAction)
        return 0;
    if (d->actions[action])
        return d->actions[action];

    QString text;
    QIcon icon;
    QStyle *style = d->view ? d->view->style() : qApp->style();

    switch (action) {
    case Back:
        text = tr("Back");
        icon = style->standardIcon(QStyle::SP_ArrowBack);
        break;
    case Forward:
        text = tr("Forward");
        icon = style->standardIcon(QStyle::SP_ArrowForward);
        break;
    case Stop:
        text = tr("Stop");
        icon = style->standardIcon(QStyle::SP_BrowserStop);
        break;
    case Reload:
        text = tr("Reload");
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    default:
        break;
    }

    QAction *a = new QAction(const_cast<QWebEnginePage*>(this));
    a->setText(text);
    a->setData(action);
    a->setIcon(icon);

    connect(a, SIGNAL(triggered(bool)), this, SLOT(_q_webActionTriggered(bool)));

    d->actions[action] = a;
    d->updateAction(action);
    return a;
}
#endif // QT_NO_ACTION

void QWebEnginePage::triggerAction(WebAction action, bool)
{
    Q_D(QWebEnginePage);
    switch (action) {
    case Back:
        d->adapter->navigateToOffset(-1);
        break;
    case Forward:
        d->adapter->navigateToOffset(1);
        break;
    case Stop:
        d->adapter->stop();
        break;
    case Reload:
        d->adapter->reload();
        break;
    default:
        Q_UNREACHABLE();
    }
}

void QWebEnginePage::setViewportSize(const QSize &size) const
{
    Q_UNUSED(size)
    Q_D(const QWebEnginePage);
    if (d->m_rwhvDelegate)
        d->m_rwhvDelegate->notifyResize();
}

bool QWebEnginePage::event(QEvent *e)
{
    Q_D(QWebEnginePage);
    if (!d->m_rwhvDelegate) {
        // FIXME: implement a signal when the render process crashes and keep track of it at this level
        // Ideally, this should be Q_ASSERT(!d->m_renderProcessLive) or something along those lines
        qWarning("%s: no render process running?\n", Q_FUNC_INFO);
        return false;
    }
    if (!d->m_rwhvDelegate->forwardEvent(e))
        return QObject::event(e);
    return true;
}

bool QWebEnginePagePrivate::contextMenuRequested(const WebEngineContextMenuData &data)
{
    if (!view)
        return false;

    QContextMenuEvent event(QContextMenuEvent::Mouse, data.pos, view->mapToGlobal(data.pos));
    switch (view->contextMenuPolicy()) {
    case Qt::PreventContextMenu:
        return false;
    case Qt::DefaultContextMenu:
        m_menuData = data;
        view->contextMenuEvent(&event);
        break;
    case Qt::CustomContextMenu:
        Q_EMIT view->customContextMenuRequested(data.pos);
        break;
    case Qt::ActionsContextMenu:
        if (view->actions().count()) {
            QMenu::exec(view->actions(), event.globalPos(), 0, view);
            break;
        }
        // fall through
    default:
        event.ignore();
        return false;
        break;
    }
    Q_ASSERT(view->d_func()->m_pendingContextMenuEvent);
    view->d_func()->m_pendingContextMenuEvent = false;
    m_menuData = WebEngineContextMenuData();
    return true;
}

void QWebEnginePagePrivate::javascriptDialog(QSharedPointer<JavaScriptDialogController> controller)
{
    Q_Q(QWebEnginePage);
    bool accepted = false;
    QString promptResult;
    switch (controller->type()) {
    case AlertDialog:
        q->javaScriptAlert(0, controller->message());
        accepted = true;
        break;
    case ConfirmDialog:
        accepted = q->javaScriptConfirm(0, controller->message());
        break;
    case PromptDialog:
        accepted = q->javaScriptPrompt(0, controller->message(), controller->defaultPrompt(), &promptResult);
        if (accepted)
            controller->textProvided(promptResult);
        break;
    default:
        Q_UNREACHABLE();
    }
    if (accepted)
        controller->accept();
    else
        controller->reject();
}

void QWebEnginePagePrivate::javaScriptConsoleMessage(int level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_Q(QWebEnginePage);
    q->javaScriptConsoleMessage(level, message, lineNumber, sourceID);
}

namespace {
class SaveToClipboardFunctor
{
    QString m_text;
public:
    SaveToClipboardFunctor(const QString &text)
      : m_text(text)
    {}
    void operator()() const
    {
        qApp->clipboard()->setText(m_text);
    }
};

class LoadUrlFunctor
{
    QWebEnginePage *m_page;
    QUrl m_url;
public:
    LoadUrlFunctor(QWebEnginePage *page, const QUrl &url)
      : m_page(page)
      , m_url(url)
    {}
    void operator()() const
    {
        m_page->load(m_url);
    }
};
}

QMenu *QWebEnginePage::createStandardContextMenu()
{
    Q_D(QWebEnginePage);
    QMenu *menu = new QMenu(d->view);
    QAction *action = 0;
    WebEngineContextMenuData contextMenuData(d->m_menuData);
    if (contextMenuData.selectedText.isEmpty()) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), tr("&Back"), menu);
        connect(action, &QAction::triggered, d->view, &QWebEngineView::back);
        action->setEnabled(d->adapter->canGoBack());
        menu->addAction(action);

        action = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), tr("&Forward"), menu);
        connect(action, &QAction::triggered, d->view, &QWebEngineView::forward);
        action->setEnabled(d->adapter->canGoForward());
        menu->addAction(action);

        action = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("&Reload"), menu);
        connect(action, &QAction::triggered, d->view, &QWebEngineView::reload);
        menu->addAction(action);
    } else {
        action = new QAction(tr("Copy..."), menu);
        connect(action, &QAction::triggered, SaveToClipboardFunctor(contextMenuData.selectedText));
        menu->addAction(action);
    }

    if (!contextMenuData.linkText.isEmpty() && contextMenuData.linkUrl.isValid()) {
        menu->addSeparator();
        action = new QAction(tr("Navigate to..."), menu);
        connect(action, &QAction::triggered, LoadUrlFunctor(this, contextMenuData.linkUrl));
        menu->addAction(action);
        action = new QAction(tr("Copy link address"), menu);
        connect(action, &QAction::triggered, SaveToClipboardFunctor(contextMenuData.linkUrl.toString()));
        menu->addAction(action);
    }
    return menu;
}

static inline QWebEnginePage::FileSelectionMode toPublic(WebContentsAdapterClient::FileChooserMode mode)
{
    // Should the underlying values change, we'll need a switch here.
    return static_cast<QWebEnginePage::FileSelectionMode>(mode);
}

void QWebEnginePagePrivate::runFileChooser(WebContentsAdapterClient::FileChooserMode mode, const QString &defaultFileName, const QStringList &acceptedMimeTypes)
{
    Q_Q(QWebEnginePage);
    QStringList selectedFileNames = q->chooseFiles(toPublic(mode), (QStringList() << defaultFileName), acceptedMimeTypes);
    adapter->filesSelectedInChooser(selectedFileNames, mode);
}

void QWebEnginePage::load(const QUrl& url)
{
    Q_D(QWebEnginePage);
    d->adapter->load(url);
}

void QWebEnginePage::toHtml(const QWebEngineCallback<const QString &> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    quint64 requestId = d->adapter->fetchDocumentMarkup();
    d->m_stringCallbacks.insert(requestId, resultCallback.d);
}

void QWebEnginePage::toPlainText(const QWebEngineCallback<const QString &> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    quint64 requestId = d->adapter->fetchDocumentInnerText();
    d->m_stringCallbacks.insert(requestId, resultCallback.d);
}

void QWebEnginePage::setHtml(const QString &html, const QUrl &baseUrl)
{
    setContent(html.toUtf8(), QStringLiteral("text/html;charset=UTF-8"), baseUrl);
}

void QWebEnginePage::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
    Q_D(QWebEnginePage);
    d->adapter->setContent(data, mimeType, baseUrl, baseUrl);
}

QString QWebEnginePage::title() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->pageTitle();
}

void QWebEnginePage::setUrl(const QUrl &url)
{
    Q_D(QWebEnginePage);
    d->m_explicitUrl = url;
    load(url);
}

QUrl QWebEnginePage::url() const
{
    Q_D(const QWebEnginePage);
    return d->m_explicitUrl.isValid() ? d->m_explicitUrl : d->adapter->activeUrl();
}

QUrl QWebEnginePage::requestedUrl() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->requestedUrl();
}

void QWebEnginePage::render(QPainter *p, const QRegion &clip)
{
    Q_D(const QWebEnginePage);
    if (!d->m_rwhvDelegate) {
        // Most likely the render process crashed. See QWebEnginePage::event
        return;
    }
    if (!clip.isNull()) {
        p->save();
        p->setClipRegion(clip);
    }
    d->m_rwhvDelegate->paint(p, QRectF(clip.boundingRect()));
    if (!clip.isNull())
        p->restore();
}

qreal QWebEnginePage::zoomFactor() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->currentZoomFactor();
}

void QWebEnginePage::setZoomFactor(qreal factor)
{
    Q_D(QWebEnginePage);
    d->adapter->setZoomFactor(factor);
}

bool QWebEnginePage::hasFocus() const
{
    Q_D(const QWebEnginePage);
    if (d->view)
        return d->view->hasFocus();
    return false;
}

void QWebEnginePage::setFocus()
{
    Q_D(QWebEnginePage);
    if (d->view)
        d->view->setFocus();
}

void QWebEnginePage::runJavaScript(const QString &scriptSource, const QString &xPath)
{
    Q_D(QWebEnginePage);
    d->adapter->runJavaScript(scriptSource, xPath);
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, const QWebEngineCallback<const QVariant &> &resultCallback, const QString &xPath)
{
    Q_D(QWebEnginePage);
    quint64 requestId = d->adapter->runJavaScriptCallbackResult(scriptSource, xPath);
    d->m_variantCallbacks.insert(requestId, resultCallback.d);
}

QWebEnginePage *QWebEnginePage::createWindow(WebWindowType type)
{
    Q_D(QWebEnginePage);
    if (d->view) {
        QWebEngineView *newView = d->view->createWindow(type);
        if (newView)
            return newView->page();
    }
    return 0;
}

Q_STATIC_ASSERT_X(static_cast<int>(WebContentsAdapterClient::Open) == static_cast<int>(QWebEnginePage::FileSelectOpen), "Enums out of sync");
Q_STATIC_ASSERT_X(static_cast<int>(WebContentsAdapterClient::OpenMultiple) == static_cast<int>(QWebEnginePage::FileSelectOpenMultiple), "Enums out of sync");

QStringList QWebEnginePage::chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
    // FIXME: Should we expose this in QWebPage's API ? Right now it is very open and can contain a mix and match of file extensions (which QFileDialog
    // can work with) and mimetypes ranging from text/plain or images/* to application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
    Q_UNUSED(acceptedMimeTypes);
    QStringList ret;
    switch (static_cast<WebContentsAdapterClient::FileChooserMode>(mode)) {
    case WebContentsAdapterClient::OpenMultiple:
        ret = QFileDialog::getOpenFileNames(view(), QString());
        break;
    // Chromium extension, not exposed as part of the public API for now.
    case WebContentsAdapterClient::UploadFolder:
        ret << QFileDialog::getExistingDirectory(view(), tr("Select folder to upload")) + QLatin1Char('/');
        break;
    case WebContentsAdapterClient::Save:
        ret << QFileDialog::getSaveFileName(view(), QString(), (QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + oldFiles.first()));
        break;
    default:
    case WebContentsAdapterClient::Open:
        ret << QFileDialog::getOpenFileName(view(), QString(), oldFiles.first());
        break;
    }
    return ret;
}

void QWebEnginePage::javaScriptAlert(QWebEngineFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    QMessageBox::information(view(), QStringLiteral("Javascript Alert - %1").arg(url().toString()), msg);
}

bool QWebEnginePage::javaScriptConfirm(QWebEngineFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    return (QMessageBox::information(view(), QStringLiteral("Javascript Confirm - %1").arg(url().toString()), msg, QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok);
}

bool QWebEnginePage::javaScriptPrompt(QWebEngineFrame *originatingFrame, const QString &msg, const QString &defaultValue, QString *result)
{
    Q_UNUSED(originatingFrame);
    bool ret = false;
    if (result)
        *result = QInputDialog::getText(view(), QStringLiteral("Javascript Prompt - %1").arg(url().toString()), msg, QLineEdit::Normal, defaultValue, &ret);
    return ret;
}

void QWebEnginePage::javaScriptConsoleMessage(int level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_UNUSED(level);
    Q_UNUSED(message);
    Q_UNUSED(lineNumber);
    Q_UNUSED(sourceID);
}
QT_END_NAMESPACE

#include "moc_qwebenginepage.cpp"
