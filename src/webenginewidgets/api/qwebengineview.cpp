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

#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include <QtWebEngineCore/private/qwebenginepage_p.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"
#include "file_picker_controller.h"
#include "qwebenginenotificationpresenter_p.h"
#include "color_chooser_controller.h"
#include <QStandardPaths>
#if QT_CONFIG(action)
#include <QAction>
#endif
#if QT_CONFIG(menu)
#include <QMenu>
#endif
#include <QContextMenuEvent>
#include <QToolTip>
#include <QVBoxLayout>
#if QT_CONFIG(colordialog)
#    include <QColorDialog>
#endif
#include <QContextMenuEvent>
#if QT_CONFIG(filedialog)
#    include <QFileDialog>
#endif
#include <QKeyEvent>
#include <QIcon>
#if QT_CONFIG(inputdialog)
#    include <QInputDialog>
#endif
#include <QLayout>
#include <QLoggingCategory>
#if QT_CONFIG(menu)
#    include <QMenu>
#endif
#if QT_CONFIG(messagebox)
#    include <QMessageBox>
#endif
#include <QStyle>
#include <QGuiApplication>

QT_BEGIN_NAMESPACE

void QWebEngineViewPrivate::pageChanged(QWebEnginePage *oldPage, QWebEnginePage *newPage)
{
    Q_Q(QWebEngineView);

    if (oldPage) {
        oldPage->setVisible(false);
        QObject::disconnect(oldPage, &QWebEnginePage::titleChanged, q, &QWebEngineView::titleChanged);
        QObject::disconnect(oldPage, &QWebEnginePage::urlChanged, q, &QWebEngineView::urlChanged);
        QObject::disconnect(oldPage, &QWebEnginePage::iconUrlChanged, q, &QWebEngineView::iconUrlChanged);
        QObject::disconnect(oldPage, &QWebEnginePage::iconChanged, q, &QWebEngineView::iconChanged);
        QObject::disconnect(oldPage, &QWebEnginePage::loadStarted, q, &QWebEngineView::loadStarted);
        QObject::disconnect(oldPage, &QWebEnginePage::loadProgress, q, &QWebEngineView::loadProgress);
        QObject::disconnect(oldPage, &QWebEnginePage::loadFinished, q, &QWebEngineView::loadFinished);
        QObject::disconnect(oldPage, &QWebEnginePage::selectionChanged, q, &QWebEngineView::selectionChanged);
        QObject::disconnect(oldPage, &QWebEnginePage::renderProcessTerminated, q, &QWebEngineView::renderProcessTerminated);
    }

    if (newPage) {
        QObject::connect(newPage, &QWebEnginePage::titleChanged, q, &QWebEngineView::titleChanged);
        QObject::connect(newPage, &QWebEnginePage::urlChanged, q, &QWebEngineView::urlChanged);
        QObject::connect(newPage, &QWebEnginePage::iconUrlChanged, q, &QWebEngineView::iconUrlChanged);
        QObject::connect(newPage, &QWebEnginePage::iconChanged, q, &QWebEngineView::iconChanged);
        QObject::connect(newPage, &QWebEnginePage::loadStarted, q, &QWebEngineView::loadStarted);
        QObject::connect(newPage, &QWebEnginePage::loadProgress, q, &QWebEngineView::loadProgress);
        QObject::connect(newPage, &QWebEnginePage::loadFinished, q, &QWebEngineView::loadFinished);
        QObject::connect(newPage, &QWebEnginePage::selectionChanged, q, &QWebEngineView::selectionChanged);
        QObject::connect(newPage, &QWebEnginePage::renderProcessTerminated, q, &QWebEngineView::renderProcessTerminated);
        newPage->setVisible(q->isVisible());
    }

    auto oldUrl = oldPage ? oldPage->url() : QUrl();
    auto newUrl = newPage ? newPage->url() : QUrl();
    if (oldUrl != newUrl)
        Q_EMIT q->urlChanged(newUrl);

    auto oldTitle = oldPage ? oldPage->title() : QString();
    auto newTitle = newPage ? newPage->title() : QString();
    if (oldTitle != newTitle)
        Q_EMIT q->titleChanged(newTitle);

    auto oldIcon = oldPage ? oldPage->iconUrl() : QUrl();
    auto newIcon = newPage ? newPage->iconUrl() : QUrl();
    if (oldIcon != newIcon) {
        Q_EMIT q->iconUrlChanged(newIcon);
        Q_EMIT q->iconChanged(newPage ? newPage->icon() : QIcon());
    }

    if ((oldPage && oldPage->hasSelection()) || (newPage && newPage->hasSelection()))
        Q_EMIT q->selectionChanged();
}

void QWebEngineViewPrivate::widgetChanged(QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *oldWidget,
                                          QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *newWidget)
{
    Q_Q(QWebEngineView);

    if (oldWidget) {
        q->layout()->removeWidget(oldWidget);
        oldWidget->hide();
#if QT_CONFIG(accessibility)
        QAccessible::deleteAccessibleInterface(QAccessible::uniqueId(QAccessible::queryAccessibleInterface(oldWidget)));
#endif
    }

    if (newWidget) {
#if QT_CONFIG(accessibility)
        // An earlier QAccessible::queryAccessibleInterface() call may have already registered a default
        // QAccessibleInterface for newWidget: remove it first to avoid assert in QAccessibleCache::insert().
        QAccessible::deleteAccessibleInterface(QAccessible::uniqueId(QAccessible::queryAccessibleInterface(newWidget)));
        QAccessible::registerAccessibleInterface(new QtWebEngineCore::RenderWidgetHostViewQtDelegateWidgetAccessible(newWidget, q));
#endif
        q->layout()->addWidget(newWidget);
        q->setFocusProxy(newWidget);
        newWidget->show();
    }
}

void QWebEngineViewPrivate::contextMenuRequested(QWebEngineContextMenuRequest *request)
{
#if QT_CONFIG(action)
    m_contextRequest = request;
    switch (q_ptr->contextMenuPolicy()) {
    case Qt::DefaultContextMenu: {
        QContextMenuEvent event(QContextMenuEvent::Mouse, request->position(),
                                q_ptr->mapToGlobal(request->position()));
        q_ptr->contextMenuEvent(&event);
        return;
    }
    case Qt::CustomContextMenu:
        Q_EMIT q_ptr->customContextMenuRequested(request->position());
        return;
    case Qt::ActionsContextMenu:
        if (q_ptr->actions().count()) {
            QContextMenuEvent event(QContextMenuEvent::Mouse, request->position(),
                                    q_ptr->mapToGlobal(request->position()));
            QMenu::exec(q_ptr->actions(), event.globalPos(), 0, q_ptr);
        }
        return;
    case Qt::PreventContextMenu:
    case Qt::NoContextMenu:
        return;
    }

    Q_UNREACHABLE();
#else
    Q_UNUSED(data);
#endif // QT_CONFIG(action)
}

QStringList QWebEngineViewPrivate::chooseFiles(QWebEnginePage::FileSelectionMode mode,
                                               const QStringList &oldFiles,
                                               const QStringList &acceptedMimeTypes)
{
#if QT_CONFIG(filedialog)
    Q_Q(QWebEngineView);
    const QStringList &filter =
            QtWebEngineCore::FilePickerController::nameFilters(acceptedMimeTypes);
    QStringList ret;
    QString str;
    switch (static_cast<QtWebEngineCore::FilePickerController::FileChooserMode>(mode)) {
    case QtWebEngineCore::FilePickerController::OpenMultiple:
        ret = QFileDialog::getOpenFileNames(q, QString(), QString(),
                                            filter.join(QStringLiteral(";;")), nullptr,
                                            QFileDialog::HideNameFilterDetails);
        break;
    // Chromium extension, not exposed as part of the public API for now.
    case QtWebEngineCore::FilePickerController::UploadFolder:
        str = QFileDialog::getExistingDirectory(q, QObject::tr("Select folder to upload"));
        if (!str.isNull())
            ret << str;
        break;
    case QtWebEngineCore::FilePickerController::Save:
        str = QFileDialog::getSaveFileName(
                q, QString(),
                (QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                 + oldFiles.first()));
        if (!str.isNull())
            ret << str;
        break;
    case QtWebEngineCore::FilePickerController::Open:
        str = QFileDialog::getOpenFileName(q, QString(), oldFiles.first(),
                                           filter.join(QStringLiteral(";;")), nullptr,
                                           QFileDialog::HideNameFilterDetails);
        if (!str.isNull())
            ret << str;
        break;
    }
    return ret;
#else
    Q_UNUSED(mode);
    Q_UNUSED(oldFiles);
    Q_UNUSED(acceptedMimeTypes);

    return QStringList();
#endif // QT_CONFIG(filedialog)
}

void QWebEngineViewPrivate::showColorDialog(
        QSharedPointer<QtWebEngineCore::ColorChooserController> controller)
{
#if QT_CONFIG(colordialog)
    Q_Q(QWebEngineView);
    QColorDialog *dialog = new QColorDialog(controller.data()->initialColor(), q);

    QColorDialog::connect(dialog, SIGNAL(colorSelected(QColor)), controller.data(),
                          SLOT(accept(QColor)));
    QColorDialog::connect(dialog, SIGNAL(rejected()), controller.data(), SLOT(reject()));

    // Delete when done
    QColorDialog::connect(dialog, SIGNAL(colorSelected(QColor)), dialog, SLOT(deleteLater()));
    QColorDialog::connect(dialog, SIGNAL(rejected()), dialog, SLOT(deleteLater()));

    dialog->open();
#else
    Q_UNUSED(controller);
#endif
}

bool QWebEngineViewPrivate::showAuthorizationDialog(const QString &title, const QString &message)
{
#if QT_CONFIG(messagebox)
    Q_Q(QWebEngineView);
    return QMessageBox::question(q, title, message, QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes;
#else
    return false;
#endif // QT_CONFIG(messagebox)
}

void QWebEngineViewPrivate::javaScriptAlert(const QUrl &url, const QString &msg)
{
#if QT_CONFIG(messagebox)
    Q_Q(QWebEngineView);
    QMessageBox::information(q, QStringLiteral("Javascript Alert - %1").arg(url.toString()),
                             msg.toHtmlEscaped());
#else
    Q_UNUSED(msg);
#endif // QT_CONFIG(messagebox)
}

bool QWebEngineViewPrivate::javaScriptConfirm(const QUrl &url, const QString &msg)
{
#if QT_CONFIG(messagebox)
    Q_Q(QWebEngineView);
    return (QMessageBox::information(q,
                                     QStringLiteral("Javascript Confirm - %1").arg(url.toString()),
                                     msg.toHtmlEscaped(), QMessageBox::Ok, QMessageBox::Cancel)
            == QMessageBox::Ok);
#else
    Q_UNUSED(msg);
    return false;
#endif // QT_CONFIG(messagebox)
}

bool QWebEngineViewPrivate::javaScriptPrompt(const QUrl &url, const QString &msg,
                                             const QString &defaultValue, QString *result)
{
#if QT_CONFIG(inputdialog)
    Q_Q(QWebEngineView);
    bool ret = false;
    if (result)
        *result = QInputDialog::getText(
                q, QStringLiteral("Javascript Prompt - %1").arg(url.toString()),
                msg.toHtmlEscaped(), QLineEdit::Normal, defaultValue.toHtmlEscaped(), &ret);
    return ret;
#else
    Q_UNUSED(msg);
    Q_UNUSED(defaultValue);
    Q_UNUSED(result);
    return false;
#endif // QT_CONFIG(inputdialog)
}

void QWebEngineViewPrivate::focusContainer()
{
    Q_Q(QWebEngineView);
    q->activateWindow();
    q->setFocus();
}

void QWebEngineViewPrivate::unhandledKeyEvent(QKeyEvent *event)
{
    Q_Q(QWebEngineView);
    if (q->parentWidget())
        QGuiApplication::sendEvent(q->parentWidget(), event);
}

bool QWebEngineViewPrivate::passOnFocus(bool reverse)
{
    Q_Q(QWebEngineView);
    return q->focusNextPrevChild(!reverse);
}

#ifndef QT_NO_ACCESSIBILITY
static QAccessibleInterface *webAccessibleFactory(const QString &, QObject *object)
{
    if (QWebEngineView *v = qobject_cast<QWebEngineView*>(object))
        return new QWebEngineViewAccessible(v);
    return nullptr;
}
#endif // QT_NO_ACCESSIBILITY

QWebEngineViewPrivate::QWebEngineViewPrivate()
    : page(0), m_dragEntered(false), m_ownsPage(false), m_contextRequest(nullptr)
{
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::installFactory(&webAccessibleFactory);
#endif // QT_NO_ACCESSIBILITY
}

QWebEngineViewPrivate::~QWebEngineViewPrivate() = default;

void QWebEngineViewPrivate::bindPageAndView(QWebEnginePage *page, QWebEngineView *view)
{
    QWebEngineViewPrivate *v =
            page ? static_cast<QWebEngineViewPrivate *>(page->d_func()->view) : nullptr;
    auto oldView = v ? v->q_func() : nullptr;
    auto oldPage = view ? view->d_func()->page : nullptr;

    bool ownNewPage = false;
    bool deleteOldPage = false;

    // Change pointers first.

    if (page && oldView != view) {
        if (oldView) {
            ownNewPage = oldView->d_func()->m_ownsPage;
            oldView->d_func()->page = nullptr;
            oldView->d_func()->m_ownsPage = false;
        }
        page->d_func()->view = view ? view->d_func() : nullptr;
    }

    if (view && oldPage != page) {
        if (oldPage) {
            if (oldPage->d_func())
                oldPage->d_func()->view = nullptr;
            deleteOldPage = view->d_func()->m_ownsPage;
        }
        view->d_func()->m_ownsPage = ownNewPage;
        view->d_func()->page = page;
    }

    // Then notify.

    auto widget = page ? page->d_func()->widget : nullptr;
    auto oldWidget = (oldPage && oldPage->d_func()) ? oldPage->d_func()->widget : nullptr;

    if (page && oldView != view && oldView) {
        oldView->d_func()->pageChanged(page, nullptr);
        if (widget)
            oldView->d_func()->widgetChanged(widget, nullptr);
    }

    if (view && oldPage != page) {
        if (oldPage && oldPage->d_func())
            view->d_func()->pageChanged(oldPage, page);
        else
            view->d_func()->pageChanged(nullptr, page);
        if (oldWidget != widget)
            view->d_func()->widgetChanged(oldWidget, widget);
    }
    if (deleteOldPage)
        delete oldPage;
}

void QWebEngineViewPrivate::bindPageAndWidget(
        QWebEnginePage *page, QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *widget)
{
    auto oldPage = widget ? widget->m_page : nullptr;
    auto oldWidget = page ? page->d_func()->widget : nullptr;

    // Change pointers first.

    if (widget && oldPage != page) {
        if (oldPage && oldPage->d_func())
            oldPage->d_func()->widget = nullptr;
        widget->m_page = page;
    }

    if (page && oldWidget != widget) {
        if (oldWidget)
            oldWidget->m_page = nullptr;
        page->d_func()->widget = widget;
    }

    // Then notify.

    if (widget && oldPage != page && oldPage && oldPage->d_func()) {
        if (auto oldView = oldPage->d_func()->view)
            static_cast<QWebEngineViewPrivate *>(oldView)->widgetChanged(widget, nullptr);
    }

    if (page && oldWidget != widget) {
        if (auto view = page->d_func()->view)
            static_cast<QWebEngineViewPrivate *>(view)->widgetChanged(oldWidget, widget);
    }
}

QIcon QWebEngineViewPrivate::webActionIcon(QWebEnginePage::WebAction action)
{
    Q_Q(QWebEngineView);
    QIcon icon;
    QStyle *style = q->style();

    switch (action) {
    case QWebEnginePage::Back:
        icon = style->standardIcon(QStyle::SP_ArrowBack);
        break;
    case QWebEnginePage::Forward:
        icon = style->standardIcon(QStyle::SP_ArrowForward);
        break;
    case QWebEnginePage::Stop:
        icon = style->standardIcon(QStyle::SP_BrowserStop);
        break;
    case QWebEnginePage::Reload:
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    case QWebEnginePage::ReloadAndBypassCache:
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    default:
        break;
    }
    return icon;
}

QWebEnginePage *QWebEngineViewPrivate::createPageForWindow(QWebEnginePage::WebWindowType type)
{
    Q_Q(QWebEngineView);
    QWebEngineView *newView = q->createWindow(type);
    if (newView)
        return newView->page();
    return nullptr;
}

void QWebEngineViewPrivate::setToolTip(const QString &toolTipText)
{
    Q_Q(QWebEngineView);
    q->setToolTip(toolTipText);
}

bool QWebEngineViewPrivate::isEnabled() const
{
    Q_Q(const QWebEngineView);
    return q->isEnabled();
}

QObject *QWebEngineViewPrivate::accessibilityParentObject()
{
    Q_Q(QWebEngineView);
    return q;
}

bool QWebEngineViewPrivate::isVisible() const
{
    Q_Q(const QWebEngineView);
    return q->isVisible();
}
QRect QWebEngineViewPrivate::viewportRect() const
{
    Q_Q(const QWebEngineView);
    return q->rect();
}
QtWebEngineCore::RenderWidgetHostViewQtDelegate *
QWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegate(
        QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client)
{
    Q_Q(QWebEngineView);
    return new QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget(client, q);
}

QWebEngineContextMenuRequest *QWebEngineViewPrivate::lastContextMenuRequest() const
{
    return m_contextRequest;
}
/*!
    \fn QWebEngineView::renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
    \since 5.6

    This signal is emitted when the render process is terminated with a non-zero exit status.
    \a terminationStatus is the termination status of the process and \a exitCode is the status code
    with which the process terminated.
*/

/*!
    \fn void QWebEngineView::iconChanged(const QIcon &icon)
    \since 5.7

    This signal is emitted when the icon ("favicon") associated with the
    view is changed. The new icon is specified by \a icon.

    \sa icon(), iconUrl(), iconUrlChanged()
*/

QWebEngineView::QWebEngineView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new QWebEngineViewPrivate)
{
    Q_D(QWebEngineView);
    d->q_ptr = this;
    setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

QWebEngineView::~QWebEngineView()
{
    blockSignals(true);
    QWebEngineViewPrivate::bindPageAndView(nullptr, this);
}

QWebEnginePage* QWebEngineView::page() const
{
    Q_D(const QWebEngineView);
    if (!d->page) {
        QWebEngineView *that = const_cast<QWebEngineView*>(this);
        that->setPage(new QWebEnginePage(that));
        d->m_ownsPage = true;
    }
    return d->page;
}

void QWebEngineView::setPage(QWebEnginePage *newPage)
{
    QWebEngineViewPrivate::bindPageAndView(newPage, this);
    connect(newPage, &QWebEnginePage::_q_aboutToDelete, this,
            [newPage]() { QWebEngineViewPrivate::bindPageAndView(newPage, nullptr); });
    auto profile = newPage->profile();
    if (!profile->notificationPresenter())
        profile->setNotificationPresenter(&defaultNotificationPresenter);
}

void QWebEngineView::load(const QUrl& url)
{
    page()->load(url);
}

/*!
    \since 5.9
    Issues the specified \a request and loads the response.

    \sa load(), setUrl(), url(), urlChanged(), QUrl::fromUserInput()
*/
void QWebEngineView::load(const QWebEngineHttpRequest &request)
{
    page()->load(request);
}

void QWebEngineView::setHtml(const QString& html, const QUrl& baseUrl)
{
    page()->setHtml(html, baseUrl);
}

void QWebEngineView::setContent(const QByteArray& data, const QString& mimeType, const QUrl& baseUrl)
{
    page()->setContent(data, mimeType, baseUrl);
}

QWebEngineHistory* QWebEngineView::history() const
{
    return page()->history();
}

QString QWebEngineView::title() const
{
    return page()->title();
}

void QWebEngineView::setUrl(const QUrl &url)
{
    page()->setUrl(url);
}

QUrl QWebEngineView::url() const
{
    return page()->url();
}

QUrl QWebEngineView::iconUrl() const
{
    return page()->iconUrl();
}

/*!
    \property QWebEngineView::icon
    \brief The icon associated with the page currently viewed.
    \since 5.7

    By default, this property contains a null icon.

    \sa iconChanged(), iconUrl(), iconUrlChanged()
*/
QIcon QWebEngineView::icon() const
{
    return page()->icon();
}

bool QWebEngineView::hasSelection() const
{
    return page()->hasSelection();
}

QString QWebEngineView::selectedText() const
{
    return page()->selectedText();
}

#ifndef QT_NO_ACTION
QAction* QWebEngineView::pageAction(QWebEnginePage::WebAction action) const
{
    return page()->action(action);
}
#endif

void QWebEngineView::triggerPageAction(QWebEnginePage::WebAction action, bool checked)
{
    page()->triggerAction(action, checked);
}

void QWebEngineView::findText(const QString &subString, QWebEnginePage::FindFlags options, const QWebEngineCallback<bool> &resultCallback)
{
    page()->findText(subString, options, resultCallback);
}

/*!
 * \reimp
 */
QSize QWebEngineView::sizeHint() const
{
    // TODO: Remove this override for Qt 6
    return QWidget::sizeHint();
}

QWebEngineSettings *QWebEngineView::settings() const
{
    return page()->settings();
}

void QWebEngineView::stop()
{
    page()->triggerAction(QWebEnginePage::Stop);
}

void QWebEngineView::back()
{
    page()->triggerAction(QWebEnginePage::Back);
}

void QWebEngineView::forward()
{
    page()->triggerAction(QWebEnginePage::Forward);
}

void QWebEngineView::reload()
{
    page()->triggerAction(QWebEnginePage::Reload);
}

QWebEngineView *QWebEngineView::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type);
    return 0;
}

qreal QWebEngineView::zoomFactor() const
{
    return page()->zoomFactor();
}

void QWebEngineView::setZoomFactor(qreal factor)
{
    page()->setZoomFactor(factor);
}

/*!
 * \reimp
 */
bool QWebEngineView::event(QEvent *ev)
{
    if (ev->type() == QEvent::ContextMenu) {
        if (contextMenuPolicy() == Qt::NoContextMenu) {
            // We forward the contextMenu event to the parent widget
            ev->ignore();
            return false;
        }

        // We swallow spontaneous contextMenu events and synthethize those back later on when we get the
        // HandleContextMenu callback from chromium
        ev->accept();
        return true;
    }

    // Override QWidget's default ToolTip handler since it doesn't hide tooltip on empty text.
    if (ev->type() == QEvent::ToolTip) {
        if (!toolTip().isEmpty())
            QToolTip::showText(static_cast<QHelpEvent *>(ev)->globalPos(), toolTip(), this, QRect(), toolTipDuration());
        else
            QToolTip::hideText();

        ev->accept();
        return true;
    }

    return QWidget::event(ev);
}

/*!
 * \reimp
 */
#if QT_CONFIG(contextmenu)
void QWebEngineView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->popup(event->globalPos());
}
#endif // QT_CONFIG(contextmenu)

/*!
 * \reimp
 */
void QWebEngineView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    page()->setVisible(true);
}

/*!
 * \reimp
 */
void QWebEngineView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    page()->setVisible(false);
}

/*!
 * \reimp
 */
void QWebEngineView::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    page()->setVisible(false);
    page()->setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
}

#if QT_CONFIG(draganddrop)
/*!
    \reimp
*/
void QWebEngineView::dragEnterEvent(QDragEnterEvent *e)
{
    Q_D(QWebEngineView);
    e->accept();
    if (d->m_dragEntered)
        d->page->d_ptr->adapter->leaveDrag();
    d->page->d_ptr->adapter->enterDrag(e, mapToGlobal(e->position().toPoint()));
    d->m_dragEntered = true;
}

/*!
    \reimp
*/
void QWebEngineView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_D(QWebEngineView);
    if (!d->m_dragEntered)
        return;
    e->accept();
    d->page->d_ptr->adapter->leaveDrag();
    d->m_dragEntered = false;
}

/*!
    \reimp
*/
void QWebEngineView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QWebEngineView);
    if (!d->m_dragEntered)
        return;
    QtWebEngineCore::WebContentsAdapter *adapter = d->page->d_ptr->adapter.data();
    Qt::DropAction dropAction =
            adapter->updateDragPosition(e, mapToGlobal(e->position().toPoint()));
    if (Qt::IgnoreAction == dropAction) {
        e->ignore();
    } else {
        e->setDropAction(dropAction);
        e->accept();
    }
}

/*!
    \reimp
*/
void QWebEngineView::dropEvent(QDropEvent *e)
{
    Q_D(QWebEngineView);
    if (!d->m_dragEntered)
        return;
    e->accept();
    d->page->d_ptr->adapter->endDragging(e, mapToGlobal(e->position().toPoint()));
    d->m_dragEntered = false;
}
#endif // QT_CONFIG(draganddrop)

#if QT_CONFIG(menu)
QMenu *QWebEngineView::createStandardContextMenu()
{
    Q_D(QWebEngineView);
    QMenu *menu = new QMenu(this);
    QContextMenuBuilder contextMenuBuilder(d->m_contextRequest, this, menu);

    contextMenuBuilder.initMenu();

    menu->setAttribute(Qt::WA_DeleteOnClose, true);

    return menu;
}
#endif // QT_CONFIG(menu)

/*!
  \since 6.0

  Returns additional data about the current context menu. It is only guaranteed to be valid during
  the call to the contextMenuEvent()

  \sa createStandardContextMenu()
*/
QWebEngineContextMenuRequest *QWebEngineView::lastContextMenuRequest() const
{
    Q_D(const QWebEngineView);
    return d->m_contextRequest;
}

#ifndef QT_NO_ACCESSIBILITY
bool QWebEngineViewAccessible::isValid() const
{
    if (!QAccessibleWidget::isValid())
        return false;

    if (!view() || !view()->d_func() || !view()->d_func()->page || !view()->d_func()->page->d_func())
        return false;

    return true;
}

QAccessibleInterface *QWebEngineViewAccessible::focusChild() const
{
    if (child(0) && child(0)->focusChild())
        return child(0)->focusChild();
    return const_cast<QWebEngineViewAccessible *>(this);
}

int QWebEngineViewAccessible::childCount() const
{
    return child(0) ? 1 : 0;
}

QAccessibleInterface *QWebEngineViewAccessible::child(int index) const
{
    if (index == 0 && isValid())
        return view()->page()->d_func()->adapter->browserAccessible();
    return nullptr;
}

int QWebEngineViewAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    if (child(0) && c == child(0))
        return 0;
    return -1;
}
#endif // QT_NO_ACCESSIBILITY

#if QT_CONFIG(action)
QContextMenuBuilder::QContextMenuBuilder(QWebEngineContextMenuRequest *request,
                                         QWebEngineView *view, QMenu *menu)
    : QtWebEngineCore::RenderViewContextMenuQt(request), m_view(view), m_menu(menu)
{
    m_view->page()->d_ptr->ensureInitialized();
}

bool QContextMenuBuilder::hasInspector()
{
    return m_view->page()->d_ptr->adapter->hasInspector();
}

bool QContextMenuBuilder::isFullScreenMode()
{
    return m_view->page()->d_ptr->isFullScreenMode();
}

void QContextMenuBuilder::addMenuItem(ContextMenuItem menuItem)
{
    QPointer<QWebEnginePage> thisRef(m_view->page());
    QAction *action = nullptr;

    switch (menuItem) {
    case ContextMenuItem::Back:
        action = thisRef->action(QWebEnginePage::Back);
        break;
    case ContextMenuItem::Forward:
        action = thisRef->action(QWebEnginePage::Forward);
        break;
    case ContextMenuItem::Reload:
        action = thisRef->action(QWebEnginePage::Reload);
        break;
    case ContextMenuItem::Cut:
        action = thisRef->action(QWebEnginePage::Cut);
        break;
    case ContextMenuItem::Copy:
        action = thisRef->action(QWebEnginePage::Copy);
        break;
    case ContextMenuItem::Paste:
        action = thisRef->action(QWebEnginePage::Paste);
        break;
    case ContextMenuItem::Undo:
        action = thisRef->action(QWebEnginePage::Undo);
        break;
    case ContextMenuItem::Redo:
        action = thisRef->action(QWebEnginePage::Redo);
        break;
    case ContextMenuItem::SelectAll:
        action = thisRef->action(QWebEnginePage::SelectAll);
        break;
    case ContextMenuItem::PasteAndMatchStyle:
        action = thisRef->action(QWebEnginePage::PasteAndMatchStyle);
        break;
    case ContextMenuItem::OpenLinkInNewWindow:
        action = thisRef->action(QWebEnginePage::OpenLinkInNewWindow);
        break;
    case ContextMenuItem::OpenLinkInNewTab:
        action = thisRef->action(QWebEnginePage::OpenLinkInNewTab);
        break;
    case ContextMenuItem::CopyLinkToClipboard:
        action = thisRef->action(QWebEnginePage::CopyLinkToClipboard);
        break;
    case ContextMenuItem::DownloadLinkToDisk:
        action = thisRef->action(QWebEnginePage::DownloadLinkToDisk);
        break;
    case ContextMenuItem::CopyImageToClipboard:
        action = thisRef->action(QWebEnginePage::CopyImageToClipboard);
        break;
    case ContextMenuItem::CopyImageUrlToClipboard:
        action = thisRef->action(QWebEnginePage::CopyImageUrlToClipboard);
        break;
    case ContextMenuItem::DownloadImageToDisk:
        action = thisRef->action(QWebEnginePage::DownloadImageToDisk);
        break;
    case ContextMenuItem::CopyMediaUrlToClipboard:
        action = thisRef->action(QWebEnginePage::CopyMediaUrlToClipboard);
        break;
    case ContextMenuItem::ToggleMediaControls:
        action = thisRef->action(QWebEnginePage::ToggleMediaControls);
        break;
    case ContextMenuItem::ToggleMediaLoop:
        action = thisRef->action(QWebEnginePage::ToggleMediaLoop);
        break;
    case ContextMenuItem::DownloadMediaToDisk:
        action = thisRef->action(QWebEnginePage::DownloadMediaToDisk);
        break;
    case ContextMenuItem::InspectElement:
        action = thisRef->action(QWebEnginePage::InspectElement);
        break;
    case ContextMenuItem::ExitFullScreen:
        action = thisRef->action(QWebEnginePage::ExitFullScreen);
        break;
    case ContextMenuItem::SavePage:
        action = thisRef->action(QWebEnginePage::SavePage);
        break;
    case ContextMenuItem::ViewSource:
        action = thisRef->action(QWebEnginePage::ViewSource);
        break;
    case ContextMenuItem::SpellingSuggestions:
        for (int i = 0; i < m_contextData->spellCheckerSuggestions().count() && i < 4; i++) {
            action = new QAction(m_menu);
            QString replacement = m_contextData->spellCheckerSuggestions().at(i);
            QObject::connect(action, &QAction::triggered, [thisRef, replacement] {
                if (thisRef)
                    thisRef->replaceMisspelledWord(replacement);
            });
            action->setText(replacement);
            m_menu->addAction(action);
        }
        return;
    case ContextMenuItem::Separator:
        if (!m_menu->isEmpty())
            m_menu->addSeparator();
        return;
    }
    action->setEnabled(isMenuItemEnabled(menuItem));
    m_menu->addAction(action);
}

bool QContextMenuBuilder::isMenuItemEnabled(ContextMenuItem menuItem)
{
    switch (menuItem) {
    case ContextMenuItem::Back:
        return m_view->page()->d_ptr->adapter->canGoBack();
    case ContextMenuItem::Forward:
        return m_view->page()->d_ptr->adapter->canGoForward();
    case ContextMenuItem::Reload:
        return true;
    case ContextMenuItem::Cut:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanCut;
    case ContextMenuItem::Copy:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanCopy;
    case ContextMenuItem::Paste:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanPaste;
    case ContextMenuItem::Undo:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanUndo;
    case ContextMenuItem::Redo:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanRedo;
    case ContextMenuItem::SelectAll:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanSelectAll;
    case ContextMenuItem::PasteAndMatchStyle:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanPaste;
    case ContextMenuItem::OpenLinkInNewWindow:
    case ContextMenuItem::OpenLinkInNewTab:
    case ContextMenuItem::CopyLinkToClipboard:
    case ContextMenuItem::DownloadLinkToDisk:
    case ContextMenuItem::CopyImageToClipboard:
    case ContextMenuItem::CopyImageUrlToClipboard:
    case ContextMenuItem::DownloadImageToDisk:
    case ContextMenuItem::CopyMediaUrlToClipboard:
    case ContextMenuItem::ToggleMediaControls:
    case ContextMenuItem::ToggleMediaLoop:
    case ContextMenuItem::DownloadMediaToDisk:
    case ContextMenuItem::InspectElement:
    case ContextMenuItem::ExitFullScreen:
    case ContextMenuItem::SavePage:
        return true;
    case ContextMenuItem::ViewSource:
        return m_view->page()->d_ptr->adapter->canViewSource();
    case ContextMenuItem::SpellingSuggestions:
    case ContextMenuItem::Separator:
        return true;
    }
    Q_UNREACHABLE();
}
#endif // QT_CONFIG(action)

QT_END_NAMESPACE

#include "moc_qwebengineview.cpp"
