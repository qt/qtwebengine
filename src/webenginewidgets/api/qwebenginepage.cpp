/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginepage.h"
#include "qwebenginepage_p.h"

#include "authentication_dialog_controller.h"
#include "browser_context_adapter.h"
#include "certificate_error_controller.h"
#include "color_chooser_controller.h"
#include "file_picker_controller.h"
#include "javascript_dialog_controller.h"
#include "qwebenginefullscreenrequest.h"
#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"
#include "qwebengineprofile.h"
#include "qwebengineprofile_p.h"
#include "qwebenginescriptcollection_p.h"
#include "qwebenginesettings.h"
#include "qwebengineview.h"
#include "qwebengineview_p.h"
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"
#include "web_engine_settings.h"

#ifdef QT_UI_DELEGATES
#include "ui/messagebubblewidget_p.h"
#endif

#include <QAction>
#include <QApplication>
#include <QAuthenticator>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QKeyEvent>
#include <QIcon>
#include <QInputDialog>
#include <QLayout>
#include <QLoggingCategory>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardPaths>
#include <QStyle>
#include <QTimer>
#include <QUrl>

#include <private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

using namespace QtWebEngineCore;

static QWebEnginePage::WebWindowType toWindowType(WebContentsAdapterClient::WindowOpenDisposition disposition)
{
    switch (disposition) {
    case WebContentsAdapterClient::NewForegroundTabDisposition:
    case WebContentsAdapterClient::NewBackgroundTabDisposition:
        return QWebEnginePage::WebBrowserTab;
    case WebContentsAdapterClient::NewPopupDisposition:
        return QWebEnginePage::WebDialog;
    case WebContentsAdapterClient::NewWindowDisposition:
        return QWebEnginePage::WebBrowserWindow;
    default:
        Q_UNREACHABLE();
    }
}

QWebEnginePage::WebAction editorActionForKeyEvent(QKeyEvent* event)
{
    static struct {
        QKeySequence::StandardKey standardKey;
        QWebEnginePage::WebAction action;
    } editorActions[] = {
        { QKeySequence::Cut, QWebEnginePage::Cut },
        { QKeySequence::Copy, QWebEnginePage::Copy },
        { QKeySequence::Paste, QWebEnginePage::Paste },
        { QKeySequence::Undo, QWebEnginePage::Undo },
        { QKeySequence::Redo, QWebEnginePage::Redo },
        { QKeySequence::SelectAll, QWebEnginePage::SelectAll },
        { QKeySequence::UnknownKey, QWebEnginePage::NoWebAction }
    };
    for (int i = 0; editorActions[i].standardKey != QKeySequence::UnknownKey; ++i)
        if (event == editorActions[i].standardKey)
            return editorActions[i].action;

    return QWebEnginePage::NoWebAction;
}

QWebEnginePagePrivate::QWebEnginePagePrivate(QWebEngineProfile *_profile)
    : adapter(QSharedPointer<WebContentsAdapter>::create())
    , history(new QWebEngineHistory(new QWebEngineHistoryPrivate(this)))
    , profile(_profile ? _profile : QWebEngineProfile::defaultProfile())
    , settings(new QWebEngineSettings(profile->settings()))
    , view(0)
    , isLoading(false)
    , scriptCollection(new QWebEngineScriptCollectionPrivate(browserContextAdapter()->userResourceController(), adapter))
    , m_isBeingAdopted(false)
    , m_backgroundColor(Qt::white)
    , fullscreenMode(false)
    , webChannel(nullptr)
{
    memset(actions, 0, sizeof(actions));
}

QWebEnginePagePrivate::~QWebEnginePagePrivate()
{
    delete history;
    delete settings;
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client)
{
    // Set the QWebEngineView as the parent for a popup delegate, so that the new popup window
    // responds properly to clicks in case the QWebEngineView is inside a modal QDialog. Setting the
    // parent essentially notifies the OS that the popup window is part of the modal session, and
    // should allow interaction.
    // The new delegate will not be deleted by the parent view though, because we unset the parent
    // when the parent is destroyed. The delegate will be destroyed by Chromium when the popup is
    // dismissed.
    // If the delegate is not for a popup, but for a newly created QWebEngineView, the parent is 0
    // just like before.
    return new RenderWidgetHostViewQtDelegateWidget(client, this->view);
}

void QWebEnginePagePrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->titleChanged(title);
}

void QWebEnginePagePrivate::urlChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    explicitUrl = QUrl();
    Q_EMIT q->urlChanged(url);
}

void QWebEnginePagePrivate::iconChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    if (iconUrl == url)
        return;
    iconUrl = url;
    Q_EMIT q->iconUrlChanged(iconUrl);
}

void QWebEnginePagePrivate::loadProgressChanged(int progress)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->loadProgress(progress);
}

void QWebEnginePagePrivate::didUpdateTargetURL(const QUrl &hoveredUrl)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->linkHovered(hoveredUrl.toString());
}

void QWebEnginePagePrivate::selectionChanged()
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->selectionChanged();
}

void QWebEnginePagePrivate::recentlyAudibleChanged(bool /*recentlyAudible*/)
{
}

QRectF QWebEnginePagePrivate::viewportRect() const
{
    return view ? view->rect() : QRectF();
}

qreal QWebEnginePagePrivate::dpiScale() const
{
    return 1.0;
}

QColor QWebEnginePagePrivate::backgroundColor() const
{
    return m_backgroundColor;
}

void QWebEnginePagePrivate::loadStarted(const QUrl &provisionalUrl, bool isErrorPage)
{
    Q_UNUSED(provisionalUrl);
    Q_Q(QWebEnginePage);

    if (isErrorPage)
        return;

    isLoading = true;
    Q_EMIT q->loadStarted();
    updateNavigationActions();
}

void QWebEnginePagePrivate::loadCommitted()
{
    updateNavigationActions();
}

void QWebEnginePagePrivate::loadFinished(bool success, const QUrl &url, bool isErrorPage, int errorCode, const QString &errorDescription)
{
    Q_Q(QWebEnginePage);
    Q_UNUSED(url);
    Q_UNUSED(errorCode);
    Q_UNUSED(errorDescription);

    if (isErrorPage) {
        Q_ASSERT(settings->testAttribute(QWebEngineSettings::ErrorPageEnabled));
        Q_ASSERT(success);
        Q_EMIT q->loadFinished(false);
        return;
    }

    isLoading = false;
    if (success)
        explicitUrl = QUrl();
    // Delay notifying failure until the error-page is done loading.
    // Error-pages are not loaded on failures due to abort.
    if (success || errorCode == -3 /* ERR_ABORTED*/ || !settings->testAttribute(QWebEngineSettings::ErrorPageEnabled)) {
        Q_EMIT q->loadFinished(success);
    }
    updateNavigationActions();
}

void QWebEnginePagePrivate::focusContainer()
{
    if (view)
        view->setFocus();
}

void QWebEnginePagePrivate::unhandledKeyEvent(QKeyEvent *event)
{
#ifdef Q_OS_OSX
    Q_Q(QWebEnginePage);
    if (event->type() == QEvent::KeyPress) {
        QWebEnginePage::WebAction action = editorActionForKeyEvent(event);
        if (action != QWebEnginePage::NoWebAction) {
            // Try triggering a registered short-cut
            if (QGuiApplicationPrivate::instance()->shortcutMap.tryShortcut(event))
                return;
            q->triggerAction(action);
            return;
        }
    }
#endif
    if (view && view->parentWidget())
        QGuiApplication::sendEvent(view->parentWidget(), event);
}

void QWebEnginePagePrivate::adoptNewWindow(QSharedPointer<WebContentsAdapter> newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect &initialGeometry)
{
    Q_Q(QWebEnginePage);
    Q_UNUSED(userGesture);

    QWebEnginePage *newPage = q->createWindow(toWindowType(disposition));
    if (!newPage)
        return;

    if (newPage->d_func() == this) {
        // If createWindow returns /this/ we must delay the adoption.
        Q_ASSERT(q == newPage);
        // WebContents might be null if we just opened a new page for navigation, in that case
        // avoid referencing newWebContents so that it is deleted and WebContentsDelegateQt::OpenURLFromTab
        // will fall back to navigating current page.
        if (newWebContents->webContents()) {
            QTimer::singleShot(0, q, [this, newPage, newWebContents, initialGeometry] () {
                adoptNewWindowImpl(newPage, newWebContents, initialGeometry);
            });
        }
    } else {
        adoptNewWindowImpl(newPage, newWebContents, initialGeometry);
    }
}

void QWebEnginePagePrivate::adoptNewWindowImpl(QWebEnginePage *newPage,
        const QSharedPointer<WebContentsAdapter> &newWebContents, const QRect &initialGeometry)
{
    // Mark the new page as being in the process of being adopted, so that a second mouse move event
    // sent by newWebContents->initialize() gets filtered in RenderWidgetHostViewQt::forwardEvent.
    // The first mouse move event is being sent by q->createWindow(). This is necessary because
    // Chromium does not get a mouse move acknowledgment message between the two events, and
    // InputRouterImpl::ProcessMouseAck is not executed, thus all subsequent mouse move events
    // get coalesced together, and don't get processed at all.
    // The mouse move events are actually sent as a result of show() being called on
    // RenderWidgetHostViewQtDelegateWidget, both when creating the window and when initialize is
    // called.
    newPage->d_func()->m_isBeingAdopted = true;

    // Overwrite the new page's WebContents with ours.
    newPage->d_func()->adapter = newWebContents;
    newWebContents->initialize(newPage->d_func());
    newPage->d_func()->scriptCollection.d->rebindToContents(newWebContents);
    if (!initialGeometry.isEmpty())
        emit newPage->geometryChangeRequested(initialGeometry);

    // If the constructor of the QWebEnginePage descendant set a web channel,
    // set it on the new adapter.
    newWebContents->setWebChannel(newPage->d_func()->webChannel, QWebEngineScript::MainWorld);

    // Page has finished the adoption process.
    newPage->d_func()->m_isBeingAdopted = false;
}

bool QWebEnginePagePrivate::isBeingAdopted()
{
    return m_isBeingAdopted;
}

void QWebEnginePagePrivate::close()
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->windowCloseRequested();
}

void QWebEnginePagePrivate::windowCloseRejected()
{
    // Do nothing for now.
}

void QWebEnginePagePrivate::didRunJavaScript(quint64 requestId, const QVariant& result)
{
    m_callbacks.invoke(requestId, result);
}

void QWebEnginePagePrivate::didFetchDocumentMarkup(quint64 requestId, const QString& result)
{
    m_callbacks.invoke(requestId, result);
}

void QWebEnginePagePrivate::didFetchDocumentInnerText(quint64 requestId, const QString& result)
{
    m_callbacks.invoke(requestId, result);
}

void QWebEnginePagePrivate::didFindText(quint64 requestId, int matchCount)
{
    m_callbacks.invoke(requestId, matchCount > 0);
}

void QWebEnginePagePrivate::didPrintPage(quint64 /*requestId*/, const QByteArray &/*result*/)
{
    Q_UNREACHABLE();
}

void QWebEnginePagePrivate::passOnFocus(bool reverse)
{
    if (view)
        view->focusNextPrevChild(!reverse);
}

void QWebEnginePagePrivate::authenticationRequired(QSharedPointer<AuthenticationDialogController> controller)
{
    Q_Q(QWebEnginePage);
    QAuthenticator networkAuth;
    networkAuth.setRealm(controller->realm());

    if (controller->isProxy())
        Q_EMIT q->proxyAuthenticationRequired(controller->url(), &networkAuth, controller->host());
    else
        Q_EMIT q->authenticationRequired(controller->url(), &networkAuth);

    // Authentication has been cancelled
    if (networkAuth.isNull()) {
        controller->reject();
        return;
    }

    controller->accept(networkAuth.user(), networkAuth.password());
}

void QWebEnginePagePrivate::showColorDialog(QSharedPointer<ColorChooserController> controller)
{
    controller->reject();
}

void QWebEnginePagePrivate::runMediaAccessPermissionRequest(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags requestFlags)
{
    Q_Q(QWebEnginePage);
    QWebEnginePage::Feature requestedFeature;
    if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture) && requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        requestedFeature = QWebEnginePage::MediaAudioVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
        requestedFeature = QWebEnginePage::MediaAudioCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        requestedFeature = QWebEnginePage::MediaVideoCapture;
    else
        return;
    Q_EMIT q->featurePermissionRequested(securityOrigin, requestedFeature);
}

void QWebEnginePagePrivate::runGeolocationPermissionRequest(const QUrl &securityOrigin)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->featurePermissionRequested(securityOrigin, QWebEnginePage::Geolocation);
}

void QWebEnginePagePrivate::runMouseLockPermissionRequest(const QUrl &securityOrigin)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->featurePermissionRequested(securityOrigin, QWebEnginePage::MouseLock);
}

#ifndef QT_NO_ACCESSIBILITY
QObject *QWebEnginePagePrivate::accessibilityParentObject()
{
    return view;
}
#endif // QT_NO_ACCESSIBILITY

void QWebEnginePagePrivate::updateAction(QWebEnginePage::WebAction action) const
{
#ifdef QT_NO_ACTION
    Q_UNUSED(action)
#else
    QAction *a = actions[action];
    if (!a)
        return;

    bool enabled = true;

    switch (action) {
    case QWebEnginePage::Back:
        enabled = adapter->canGoBack();
        break;
    case QWebEnginePage::Forward:
        enabled = adapter->canGoForward();
        break;
    case QWebEnginePage::Stop:
        enabled = isLoading;
        break;
    case QWebEnginePage::Reload:
    case QWebEnginePage::ReloadAndBypassCache:
        enabled = !isLoading;
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

void QWebEnginePagePrivate::recreateFromSerializedHistory(QDataStream &input)
{
    QSharedPointer<WebContentsAdapter> newWebContents = WebContentsAdapter::createFromSerializedNavigationHistory(input, this);
    if (newWebContents) {
        adapter = std::move(newWebContents);
        adapter->initialize(this);
        if (webChannel)
            adapter->setWebChannel(webChannel, QWebEngineScript::MainWorld);
        scriptCollection.d->rebindToContents(adapter);
    }
}

void QWebEnginePagePrivate::updateScrollPosition(const QPointF &/*position*/)
{
}

void QWebEnginePagePrivate::updateContentsSize(const QSizeF &/*size*/)
{
}

void QWebEnginePagePrivate::setFullScreenMode(bool fullscreen)
{
    if (fullscreenMode != fullscreen) {
        fullscreenMode = fullscreen;
        adapter->changedFullScreen();
    }
}

QSharedPointer<BrowserContextAdapter> QWebEnginePagePrivate::browserContextAdapter()
{
    return profile->d_ptr->browserContext();
}

WebContentsAdapter *QWebEnginePagePrivate::webContentsAdapter()
{
    return adapter.data();
}

const QObject *QWebEnginePagePrivate::holdingQObject() const
{
    Q_Q(const QWebEnginePage);
    return q;
}

QWebEnginePage::QWebEnginePage(QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEnginePagePrivate())
{
    Q_D(QWebEnginePage);
    d->q_ptr = this;
    d->adapter->initialize(d);
}

/*!
    \enum QWebEnginePage::RenderProcessTerminationStatus
    \since 5.6

    This enum describes the status with which the render process terminated:

    \value  NormalTerminationStatus
            The render process terminated normally.
    \value  AbnormalTerminationStatus
            The render process terminated with with a non-zero exit status.
    \value  CrashedTerminationStatus
            The render process crashed, for example because of a segmentation fault.
    \value  KilledTerminationStatus
            The render process was killed, for example by \c SIGKILL or task manager kill.
*/

/*!
    \fn QWebEnginePage::renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode)
    \since 5.6

    This signal is emitted when the render process is terminated with a non-zero exit status.
    \a terminationStatus is the termination status of the process and \a exitCode is the status code
    with which the process terminated.
*/

/*!
    \fn QWebEnginePage::fullScreenRequested(QWebEngineFullScreenRequest request)

    This signal is emitted when the web page issues the request to enter fullscreen mode for
    a web-element, usually a video element.

    The request object \a request can be used to accept or reject the request.

    If the request is accepted the element requesting fullscreen will fill the viewport,
    but it is up to the application to make the view fullscreen or move the page to a view
    that is fullscreen.

    \sa QWebEngineSettings::FullScreenSupportEnabled
*/

/*!
    \fn void QWebEnginePage::iconUrlChanged(const QUrl &url)

    This signal is emitted when the URL of the icon ("favicon") associated with the
    page is changed. The new URL is specified by \a url.

    \sa iconUrl()
*/

/*!
    Constructs an empty web engine page in the web engine profile \a profile with the parent
    \a parent.

    If the profile is not the default profile, the caller must ensure that the profile stays alive
    for as long as the page does.

    \since 5.5
*/
QWebEnginePage::QWebEnginePage(QWebEngineProfile *profile, QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEnginePagePrivate(profile))
{
    Q_D(QWebEnginePage);
    d->q_ptr = this;
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

QWebEngineSettings *QWebEnginePage::settings() const
{
    Q_D(const QWebEnginePage);
    return d->settings;
}

/*!
 * Returns a pointer to the web channel instance used by this page or a null pointer if none was set.
 * This channel automatically uses the internal web engine transport mechanism over Chromium IPC
 * that is exposed in the JavaScript context of this page as \c qt.webChannelTransport.
 *
 * \since 5.5
 * \sa setWebChannel
 */
QWebChannel *QWebEnginePage::webChannel() const
{
    Q_D(const QWebEnginePage);
    return d->webChannel;
}


/*!
 * Sets the web channel instance to be used by this page to \a channel and connects it to
 * web engine's transport using Chromium IPC messages. The transport is exposed in the JavaScript
 * context of this page as
 * \c qt.webChannelTransport, which should be used when using the \l{Qt WebChannel JavaScript API}.
 *
 * \note The page does not take ownership of the channel object.
 *
 * \since 5.5
 */
void QWebEnginePage::setWebChannel(QWebChannel *channel)
{
    Q_D(QWebEnginePage);
    if (d->webChannel != channel) {
        d->webChannel = channel;
        d->adapter->setWebChannel(channel, QWebEngineScript::MainWorld);
    }
}

/*!
    \property QWebEnginePage::backgroundColor
    \brief the page's background color behind the document's body.
    \since 5.6

    You can set the background color to Qt::transparent or to a translucent
    color to see through the document, or you can set it to match your
    web content in a hybrid application to prevent the white flashes that may appear
    during loading.

    The default value is white.
*/
QColor QWebEnginePage::backgroundColor() const
{
    Q_D(const QWebEnginePage);
    return d->m_backgroundColor;
}

void QWebEnginePage::setBackgroundColor(const QColor &color)
{
    Q_D(QWebEnginePage);
    if (d->m_backgroundColor == color)
        return;
    d->m_backgroundColor = color;
    d->adapter->backgroundColorChanged();
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

/*!
    Returns the web engine profile the page belongs to.
    \since 5.5
*/
QWebEngineProfile *QWebEnginePage::profile() const
{
    Q_D(const QWebEnginePage);
    return d->profile;
}

bool QWebEnginePage::hasSelection() const
{
    return !selectedText().isEmpty();
}

QString QWebEnginePage::selectedText() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->selectedText();
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
    case ReloadAndBypassCache:
        text = tr("Reload and Bypass Cache");
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    case Cut:
        text = tr("Cut");
        break;
    case Copy:
        text = tr("Copy");
        break;
    case Paste:
        text = tr("Paste");
        break;
    case Undo:
        text = tr("Undo");
        break;
    case Redo:
        text = tr("Redo");
        break;
    case SelectAll:
        text = tr("Select All");
        break;
    case PasteAndMatchStyle:
        text = tr("Paste and Match Style");
        break;
    case OpenLinkInThisWindow:
        text = tr("Open Link in This Window");
        break;
    case OpenLinkInNewWindow:
        text = tr("Open Link in New Window");
        break;
    case OpenLinkInNewTab:
        text = tr("Open Link in New Tab");
        break;
    case CopyLinkToClipboard:
        text = tr("Copy Link URL");
        break;
    case DownloadLinkToDisk:
        text = tr("Save Link");
        break;
    case CopyImageToClipboard:
        text = tr("Copy Image");
        break;
    case CopyImageUrlToClipboard:
        text = tr("Copy Image URL");
        break;
    case DownloadImageToDisk:
        text = tr("Save Image");
        break;
    case CopyMediaUrlToClipboard:
        text = tr("Copy Media URL");
        break;
    case ToggleMediaControls:
        text = tr("Toggle Media Controls");
        break;
    case ToggleMediaLoop:
        text = tr("Toggle Looping");
        break;
    case ToggleMediaPlayPause:
        text = tr("Toggle Play/Pause");
        break;
    case ToggleMediaMute:
        text = tr("Toggle Mute");
        break;
    case DownloadMediaToDisk:
        text = tr("Save Media");
        break;
    case InspectElement:
        text = tr("Inspect Element");
        break;
    case ExitFullScreen:
        text = tr("Exit Full Screen Mode");
        break;
    case RequestClose:
        text = tr("Close Page");
        break;
    case NoWebAction:
    case WebActionCount:
        Q_UNREACHABLE();
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
    const QtWebEngineCore::WebEngineContextMenuData &menuData = d->m_menuData;
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
    case ReloadAndBypassCache:
        d->adapter->reloadAndBypassCache();
        break;
    case Cut:
        d->adapter->cut();
        break;
    case Copy:
        d->adapter->copy();
        break;
    case Paste:
        d->adapter->paste();
        break;
    case Undo:
        d->adapter->undo();
        break;
    case Redo:
        d->adapter->redo();
        break;
    case SelectAll:
        d->adapter->selectAll();
        break;
    case PasteAndMatchStyle:
        d->adapter->pasteAndMatchStyle();
        break;
    case OpenLinkInThisWindow:
        if (menuData.linkUrl.isValid())
            setUrl(menuData.linkUrl);
        break;
    case OpenLinkInNewWindow:
        if (menuData.linkUrl.isValid()) {
            QWebEnginePage *newPage = createWindow(WebBrowserWindow);
            if (newPage)
                newPage->setUrl(menuData.linkUrl);
        }
        break;
    case OpenLinkInNewTab:
        if (menuData.linkUrl.isValid()) {
            QWebEnginePage *newPage = createWindow(WebBrowserTab);
            if (newPage)
                newPage->setUrl(menuData.linkUrl);
        }
        break;
    case CopyLinkToClipboard:
        if (menuData.linkUrl.isValid()) {
            QString urlString = menuData.linkUrl.toString(QUrl::FullyEncoded);
            QString title = menuData.linkText.toHtmlEscaped();
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<a href=\"") + urlString + QStringLiteral("\">") + title + QStringLiteral("</a>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << menuData.linkUrl);
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case DownloadLinkToDisk:
        if (menuData.linkUrl.isValid())
            d->adapter->download(menuData.linkUrl, menuData.suggestedFileName);
        break;
    case CopyImageToClipboard:
        if (menuData.hasImageContent &&
                (menuData.mediaType == WebEngineContextMenuData::MediaTypeImage ||
                 menuData.mediaType == WebEngineContextMenuData::MediaTypeCanvas))
        {
            d->adapter->copyImageAt(menuData.pos);
        }
        break;
    case CopyImageUrlToClipboard:
        if (menuData.mediaUrl.isValid() && menuData.mediaType == WebEngineContextMenuData::MediaTypeImage) {
            QString urlString = menuData.mediaUrl.toString(QUrl::FullyEncoded);
            QString title = menuData.linkText;
            if (!title.isEmpty())
                title = QStringLiteral(" alt=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<img src=\"") + urlString + QStringLiteral("\"") + title + QStringLiteral("></img>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << menuData.mediaUrl);
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case DownloadImageToDisk:
    case DownloadMediaToDisk:
        if (menuData.mediaUrl.isValid())
            d->adapter->download(menuData.mediaUrl, menuData.suggestedFileName);
        break;
    case CopyMediaUrlToClipboard:
        if (menuData.mediaUrl.isValid() &&
                (menuData.mediaType == WebEngineContextMenuData::MediaTypeAudio ||
                 menuData.mediaType == WebEngineContextMenuData::MediaTypeVideo))
        {
            QString urlString = menuData.mediaUrl.toString(QUrl::FullyEncoded);
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            if (menuData.mediaType == WebEngineContextMenuData::MediaTypeAudio)
                data->setHtml(QStringLiteral("<audio src=\"") + urlString + QStringLiteral("\"></audio>"));
            else
                data->setHtml(QStringLiteral("<video src=\"") + urlString + QStringLiteral("\"></video>"));
            data->setUrls(QList<QUrl>() << menuData.mediaUrl);
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case ToggleMediaControls:
        if (menuData.mediaUrl.isValid() && menuData.mediaFlags & WebEngineContextMenuData::MediaCanToggleControls) {
            bool enable = !(menuData.mediaFlags & WebEngineContextMenuData::MediaControls);
            d->adapter->executeMediaPlayerActionAt(menuData.pos, WebContentsAdapter::MediaPlayerControls, enable);
        }
        break;
    case ToggleMediaLoop:
        if (menuData.mediaUrl.isValid() &&
                (menuData.mediaType == WebEngineContextMenuData::MediaTypeAudio ||
                 menuData.mediaType == WebEngineContextMenuData::MediaTypeVideo))
        {
            bool enable = !(menuData.mediaFlags & WebEngineContextMenuData::MediaLoop);
            d->adapter->executeMediaPlayerActionAt(menuData.pos, WebContentsAdapter::MediaPlayerLoop, enable);
        }
        break;
    case ToggleMediaPlayPause:
        if (menuData.mediaUrl.isValid() &&
                (menuData.mediaType == WebEngineContextMenuData::MediaTypeAudio ||
                 menuData.mediaType == WebEngineContextMenuData::MediaTypeVideo))
        {
            bool enable = (menuData.mediaFlags & WebEngineContextMenuData::MediaPaused);
            d->adapter->executeMediaPlayerActionAt(menuData.pos, WebContentsAdapter::MediaPlayerPlay, enable);
        }
        break;
    case ToggleMediaMute:
        if (menuData.mediaUrl.isValid() && menuData.mediaFlags & WebEngineContextMenuData::MediaHasAudio) {
            // Make sure to negate the value, so that toggling actually works.
            bool enable = !(menuData.mediaFlags & WebEngineContextMenuData::MediaMuted);
            d->adapter->executeMediaPlayerActionAt(menuData.pos, WebContentsAdapter::MediaPlayerMute, enable);
        }
        break;
    case InspectElement:
        d->adapter->inspectElementAt(menuData.pos);
        break;
    case ExitFullScreen:
        d->adapter->exitFullScreen();
        break;
    case RequestClose:
        d->adapter->requestClose();
        break;
    case NoWebAction:
        break;
    case WebActionCount:
        Q_UNREACHABLE();
        break;
    }
}

void QWebEnginePage::findText(const QString &subString, FindFlags options, const QWebEngineCallback<bool> &resultCallback)
{
    Q_D(QWebEnginePage);
    if (subString.isEmpty()) {
        d->adapter->stopFinding();
        d->m_callbacks.invokeEmpty(resultCallback);
    } else {
        quint64 requestId = d->adapter->findText(subString, options & FindCaseSensitively, options & FindBackward);
        d->m_callbacks.registerCallback(requestId, resultCallback);
    }
}

/*!
 * \reimp
 */
bool QWebEnginePage::event(QEvent *e)
{
    return QObject::event(e);
}

void QWebEnginePagePrivate::wasShown()
{
    adapter->wasShown();
}

void QWebEnginePagePrivate::wasHidden()
{
    adapter->wasHidden();
}

bool QWebEnginePagePrivate::contextMenuRequested(const WebEngineContextMenuData &data)
{
    if (!view || !view->d_func()->m_pendingContextMenuEvent)
        return false;

    m_menuData = WebEngineContextMenuData();
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
        // fallthrough
    case Qt::NoContextMenu:
        event.ignore();
        return false;
    }
    view->d_func()->m_pendingContextMenuEvent = false;
    return true;
}

void QWebEnginePagePrivate::navigationRequested(int navigationType, const QUrl &url, int &navigationRequestAction, bool isMainFrame)
{
    Q_Q(QWebEnginePage);
    bool accepted = q->acceptNavigationRequest(url, static_cast<QWebEnginePage::NavigationType>(navigationType), isMainFrame);
    navigationRequestAction = accepted ? WebContentsAdapterClient::AcceptRequest : WebContentsAdapterClient::IgnoreRequest;
}

void QWebEnginePagePrivate::requestFullScreenMode(const QUrl &origin, bool fullscreen)
{
    Q_Q(QWebEnginePage);
    QWebEngineFullScreenRequest request(q, origin, fullscreen);
    Q_EMIT q->fullScreenRequested(request);
}

bool QWebEnginePagePrivate::isFullScreenMode() const
{
    return fullscreenMode;
}

void QWebEnginePagePrivate::javascriptDialog(QSharedPointer<JavaScriptDialogController> controller)
{
    Q_Q(QWebEnginePage);
    bool accepted = false;
    QString promptResult;
    switch (controller->type()) {
    case AlertDialog:
        q->javaScriptAlert(controller->securityOrigin(), controller->message());
        accepted = true;
        break;
    case ConfirmDialog:
        accepted = q->javaScriptConfirm(controller->securityOrigin(), controller->message());
        break;
    case PromptDialog:
        accepted = q->javaScriptPrompt(controller->securityOrigin(), controller->message(), controller->defaultPrompt(), &promptResult);
        if (accepted)
            controller->textProvided(promptResult);
        break;
    case UnloadDialog:
        accepted = (QMessageBox::information(view, QCoreApplication::translate("QWebEnginePage", "Are you sure you want to leave this page?"), controller->message(), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
        break;
    case InternalAuthorizationDialog:
        accepted = (QMessageBox::question(view, controller->title(), controller->message(), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
        break;
    }
    if (accepted)
        controller->accept();
    else
        controller->reject();
}

void QWebEnginePagePrivate::allowCertificateError(const QSharedPointer<CertificateErrorController> &controller)
{
    Q_Q(QWebEnginePage);
    bool accepted = false;

    QWebEngineCertificateError error(controller->error(), controller->url(), controller->overridable() && !controller->strictEnforcement(), controller->errorString());
    accepted = q->certificateError(error);

    if (error.isOverridable())
        controller->accept(accepted);
}

void QWebEnginePagePrivate::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_Q(QWebEnginePage);
    q->javaScriptConsoleMessage(static_cast<QWebEnginePage::JavaScriptConsoleMessageLevel>(level), message, lineNumber, sourceID);
}

void QWebEnginePagePrivate::showValidationMessage(const QRect &anchor, const QString &mainText, const QString &subText)
{
#ifdef QT_UI_DELEGATES
    QtWebEngineWidgetUI::MessageBubbleWidget::showBubble(view, anchor, mainText, subText);
#endif
}

void QWebEnginePagePrivate::hideValidationMessage()
{
#ifdef QT_UI_DELEGATES
    QtWebEngineWidgetUI::MessageBubbleWidget::hideBubble();
#endif
}

void QWebEnginePagePrivate::moveValidationMessage(const QRect &anchor)
{
#ifdef QT_UI_DELEGATES
    QtWebEngineWidgetUI::MessageBubbleWidget::moveBubble(view, anchor);
#endif
}

void QWebEnginePagePrivate::renderProcessTerminated(RenderProcessTerminationStatus terminationStatus,
                                                int exitCode)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->renderProcessTerminated(static_cast<QWebEnginePage::RenderProcessTerminationStatus>(
                                      terminationStatus), exitCode);
}

void QWebEnginePagePrivate::requestGeometryChange(const QRect &geometry)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->geometryChangeRequested(geometry);
}

void QWebEnginePagePrivate::startDragging(const content::DropData &dropData,
                                          Qt::DropActions allowedActions, const QPixmap &pixmap,
                                          const QPoint &offset)
{
    adapter->startDragging(view, dropData, allowedActions, pixmap, offset);
}

bool QWebEnginePagePrivate::isEnabled() const
{
    const Q_Q(QWebEnginePage);
    const QWidget *view = q->view();
    if (view)
        return view->isEnabled();
    return true;
}

QMenu *QWebEnginePage::createStandardContextMenu()
{
    Q_D(QWebEnginePage);
    QMenu *menu = new QMenu(d->view);
    QAction *action = 0;
    const WebEngineContextMenuData &contextMenuData = d->m_menuData;

    if (!contextMenuData.linkText.isEmpty() && contextMenuData.linkUrl.isValid()) {
        action = QWebEnginePage::action(OpenLinkInThisWindow);
        action->setText(tr("Follow Link"));
        menu->addAction(action);
        menu->addAction(QWebEnginePage::action(DownloadLinkToDisk));
    }
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
        menu->addAction(QWebEnginePage::action(Copy));
    }

    if (!contextMenuData.linkText.isEmpty() && contextMenuData.linkUrl.isValid()) {
        menu->addAction(QWebEnginePage::action(CopyLinkToClipboard));
    }
    if (contextMenuData.mediaUrl.isValid()) {
        switch (contextMenuData.mediaType) {
        case WebEngineContextMenuData::MediaTypeImage:
            menu->addAction(QWebEnginePage::action(DownloadImageToDisk));
            menu->addAction(QWebEnginePage::action(CopyImageUrlToClipboard));
            menu->addAction(QWebEnginePage::action(CopyImageToClipboard));
            break;
        case WebEngineContextMenuData::MediaTypeCanvas:
            Q_UNREACHABLE();    // mediaUrl is invalid for canvases
            break;
        case WebEngineContextMenuData::MediaTypeAudio:
        case WebEngineContextMenuData::MediaTypeVideo:
            menu->addAction(QWebEnginePage::action(DownloadMediaToDisk));
            menu->addAction(QWebEnginePage::action(CopyMediaUrlToClipboard));
            menu->addAction(QWebEnginePage::action(ToggleMediaPlayPause));
            menu->addAction(QWebEnginePage::action(ToggleMediaLoop));
            if (contextMenuData.mediaFlags & WebEngineContextMenuData::MediaHasAudio)
                menu->addAction(QWebEnginePage::action(ToggleMediaMute));
            if (contextMenuData.mediaFlags & WebEngineContextMenuData::MediaCanToggleControls)
                menu->addAction(QWebEnginePage::action(ToggleMediaControls));
            break;
        default:
            break;
        }
    } else if (contextMenuData.mediaType == WebEngineContextMenuData::MediaTypeCanvas) {
        menu->addAction(QWebEnginePage::action(CopyImageToClipboard));
    }

    if (d->adapter->hasInspector())
        menu->addAction(QWebEnginePage::action(InspectElement));

    if (d->isFullScreenMode())
        menu->addAction(QWebEnginePage::action(ExitFullScreen));

    return menu;
}

void QWebEnginePage::setFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature, QWebEnginePage::PermissionPolicy policy)
{
    Q_D(QWebEnginePage);
    if (policy == PermissionUnknown)
        return;
    WebContentsAdapterClient::MediaRequestFlags flags =  WebContentsAdapterClient::MediaNone;
    switch (feature) {
    case MediaAudioVideoCapture:
    case MediaAudioCapture:
    case MediaVideoCapture:
        if (policy != PermissionUnknown) {
            if (policy == PermissionDeniedByUser)
                flags = WebContentsAdapterClient::MediaNone;
            else {
                if (feature == MediaAudioCapture)
                    flags = WebContentsAdapterClient::MediaAudioCapture;
                else if (feature == MediaVideoCapture)
                    flags = WebContentsAdapterClient::MediaVideoCapture;
                else
                    flags = WebContentsAdapterClient::MediaRequestFlags(WebContentsAdapterClient::MediaVideoCapture | WebContentsAdapterClient::MediaAudioCapture);
            }
            d->adapter->grantMediaAccessPermission(securityOrigin, flags);
        }
        d->adapter->grantMediaAccessPermission(securityOrigin, flags);
        break;
    case QWebEnginePage::Geolocation:
        d->adapter->runGeolocationRequestCallback(securityOrigin, (policy == PermissionGrantedByUser) ? true : false);
        break;
    case MouseLock:
        if (policy == PermissionGrantedByUser)
            d->adapter->grantMouseLockPermission(true);
        else
            d->adapter->grantMouseLockPermission(false);
        break;
    case Notifications:
        break;
    }
}

static inline QWebEnginePage::FileSelectionMode toPublic(FilePickerController::FileChooserMode mode)
{
    // Should the underlying values change, we'll need a switch here.
    return static_cast<QWebEnginePage::FileSelectionMode>(mode);
}

void QWebEnginePagePrivate::runFileChooser(FilePickerController *controller)
{
    Q_Q(QWebEnginePage);

    QStringList selectedFileNames = q->chooseFiles(toPublic(controller->mode()), (QStringList() << controller->defaultFileName()), controller->acceptedMimeTypes());

    if (!selectedFileNames.empty())
        controller->accepted(selectedFileNames);
    else
        controller->rejected();

    delete controller;
}

WebEngineSettings *QWebEnginePagePrivate::webEngineSettings() const
{
    return settings->d_func();
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
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

void QWebEnginePage::toPlainText(const QWebEngineCallback<const QString &> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    quint64 requestId = d->adapter->fetchDocumentInnerText();
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

void QWebEnginePage::setHtml(const QString &html, const QUrl &baseUrl)
{
    setContent(html.toUtf8(), QStringLiteral("text/html;charset=UTF-8"), baseUrl);
}

void QWebEnginePage::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
    Q_D(QWebEnginePage);
    d->adapter->setContent(data, mimeType, baseUrl);
}

QString QWebEnginePage::title() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->pageTitle();
}

void QWebEnginePage::setUrl(const QUrl &url)
{
    Q_D(QWebEnginePage);
    d->explicitUrl = url;
    load(url);
}

QUrl QWebEnginePage::url() const
{
    Q_D(const QWebEnginePage);
    return d->explicitUrl.isValid() ? d->explicitUrl : d->adapter->activeUrl();
}

QUrl QWebEnginePage::requestedUrl() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->requestedUrl();
}

/*!
    \property QWebEnginePage::iconUrl
    \brief the URL of the icon associated with the page currently viewed

    By default, this property contains an empty URL.

    \sa iconUrlChanged()
*/
QUrl QWebEnginePage::iconUrl() const
{
    Q_D(const QWebEnginePage);
    return d->iconUrl;
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

void QWebEnginePage::runJavaScript(const QString &scriptSource)
{
    Q_D(QWebEnginePage);
    d->adapter->runJavaScript(scriptSource, QWebEngineScript::MainWorld);
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, const QWebEngineCallback<const QVariant &> &resultCallback)
{
    Q_D(QWebEnginePage);
    quint64 requestId = d->adapter->runJavaScriptCallbackResult(scriptSource, QWebEngineScript::MainWorld);
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

/*!
    Returns the collection of scripts that are injected into the page.

    In addition, a page might also execute scripts
    added through QWebEngineProfile::scripts().

    \sa QWebEngineScriptCollection, QWebEngineScript
*/

QWebEngineScriptCollection &QWebEnginePage::scripts()
{
    Q_D(QWebEnginePage);
    return d->scriptCollection;
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

ASSERT_ENUMS_MATCH(FilePickerController::Open, QWebEnginePage::FileSelectOpen)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, QWebEnginePage::FileSelectOpenMultiple)

QStringList QWebEnginePage::chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
    // FIXME: Should we expose this in QWebPage's API ? Right now it is very open and can contain a mix and match of file extensions (which QFileDialog
    // can work with) and mimetypes ranging from text/plain or images/* to application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
    Q_UNUSED(acceptedMimeTypes);
    QStringList ret;
    QString str;
    switch (static_cast<FilePickerController::FileChooserMode>(mode)) {
    case FilePickerController::OpenMultiple:
        ret = QFileDialog::getOpenFileNames(view(), QString());
        break;
    // Chromium extension, not exposed as part of the public API for now.
    case FilePickerController::UploadFolder:
        str = QFileDialog::getExistingDirectory(view(), tr("Select folder to upload")) + QLatin1Char('/');
        if (!str.isNull())
            ret << str;
        break;
    case FilePickerController::Save:
        str = QFileDialog::getSaveFileName(view(), QString(), (QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + oldFiles.first()));
        if (!str.isNull())
            ret << str;
        break;
    case FilePickerController::Open:
        str = QFileDialog::getOpenFileName(view(), QString(), oldFiles.first());
        if (!str.isNull())
            ret << str;
        break;
    }
    return ret;
}

void QWebEnginePage::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
    QMessageBox::information(view(), QStringLiteral("Javascript Alert - %1").arg(url().toString()), msg);
}

bool QWebEnginePage::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
    return (QMessageBox::information(view(), QStringLiteral("Javascript Confirm - %1").arg(url().toString()), msg, QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok);
}

bool QWebEnginePage::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString *result)
{
    Q_UNUSED(securityOrigin);
    bool ret = false;
    if (result)
        *result = QInputDialog::getText(view(), QStringLiteral("Javascript Prompt - %1").arg(url().toString()), msg, QLineEdit::Normal, defaultValue, &ret);
    return ret;
}

void QWebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    static QLoggingCategory loggingCategory("js", QtWarningMsg);
    static QByteArray file = sourceID.toUtf8();
    QMessageLogger logger(file.constData(), lineNumber, nullptr, loggingCategory.categoryName());

    switch (level) {
    case JavaScriptConsoleMessageLevel::InfoMessageLevel:
        if (loggingCategory.isInfoEnabled())
            logger.info().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::WarningMessageLevel:
        if (loggingCategory.isWarningEnabled())
            logger.warning().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::ErrorMessageLevel:
        if (loggingCategory.isCriticalEnabled())
            logger.critical().noquote() << message;
        break;
    }
}

bool QWebEnginePage::certificateError(const QWebEngineCertificateError &)
{
    return false;
}

bool QWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(url);
    Q_UNUSED(type);
    Q_UNUSED(isMainFrame);
    return true;
}

QT_END_NAMESPACE

#include "moc_qwebenginepage.cpp"
