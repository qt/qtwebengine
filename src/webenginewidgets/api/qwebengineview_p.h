// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEVIEW_P_H
#define QWEBENGINEVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWebEngineCore/private/qwebenginepage_p.h> // PageView

#include "render_view_context_menu_qt.h"

namespace QtWebEngineCore {
class AutofillPopupController;
class QWebEngineContextMenuRequest;
class WebEngineQuickWidget;
class RenderWidgetHostViewQtDelegate;
class RenderWidgetHostViewQtDelegateClient;
class TouchSelectionMenuController;
}

namespace QtWebEngineWidgetUI {
class AutofillPopupWidget;
class TouchHandleDrawableDelegate;
class TouchSelectionMenuWidget;
}

QT_BEGIN_NAMESPACE

class QMenu;
class QPrinter;
class QWebEngineView;

class QWebEngineViewPrivate : public PageView
{
public:
    Q_DECLARE_PUBLIC(QWebEngineView)
    QWebEngineView *q_ptr;

    void pageChanged(QWebEnginePage *oldPage, QWebEnginePage *newPage);
    void widgetChanged(QtWebEngineCore::WebEngineQuickWidget *oldWidget,
                       QtWebEngineCore::WebEngineQuickWidget *newWidget);

    void contextMenuRequested(QWebEngineContextMenuRequest *request) override;
    QStringList chooseFiles(QWebEnginePage::FileSelectionMode mode, const QStringList &oldFiles,
                            const QStringList &acceptedMimeTypes) override;
    void
    showColorDialog(QSharedPointer<QtWebEngineCore::ColorChooserController> controller) override;
    bool showAuthorizationDialog(const QString &title, const QString &message) override;
    void javaScriptAlert(const QUrl &url, const QString &msg) override;
    bool javaScriptConfirm(const QUrl &url, const QString &msg) override;
    bool javaScriptPrompt(const QUrl &url, const QString &msg, const QString &defaultValue,
                          QString *result) override;
    void setToolTip(const QString &toolTipText) override;
    QtWebEngineCore::RenderWidgetHostViewQtDelegate *CreateRenderWidgetHostViewQtDelegate(
            QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    QtWebEngineCore::RenderWidgetHostViewQtDelegate *CreateRenderWidgetHostViewQtDelegateForPopup(
            QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    QWebEngineContextMenuRequest *lastContextMenuRequest() const override;
    QWebEnginePage *createPageForWindow(QWebEnginePage::WebWindowType type) override;
    QObject *accessibilityParentObject() override;
    void didPrintPage(QPrinter *&printer, QSharedPointer<QByteArray> result) override;
    void didPrintPageToPdf(const QString &filePath, bool success) override;
    void printRequested() override;
    void showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller,
                           const QRect &bounds, bool autoselectFirstSuggestion) override;
    void hideAutofillPopup() override;
    QtWebEngineCore::TouchHandleDrawableDelegate *
    createTouchHandleDelegate(const QMap<int, QImage> &images) override;

    void showTouchSelectionMenu(QtWebEngineCore::TouchSelectionMenuController *,
                                const QRect &) override;
    void hideTouchSelectionMenu() override;
    QWebEngineViewPrivate();
    virtual ~QWebEngineViewPrivate();
    static void bindPageAndView(QWebEnginePage *page, QWebEngineView *view);
    static void bindPageAndWidget(QWebEnginePagePrivate *pagePrivate,
                                  QtWebEngineCore::WebEngineQuickWidget *widget);
    QIcon webActionIcon(QWebEnginePage::WebAction action);
    void unhandledKeyEvent(QKeyEvent *event) override;
    void focusContainer() override;
    bool passOnFocus(bool reverse) override;
    bool isEnabled() const override;
    bool isVisible() const override;
    QRect viewportRect() const override;
    QWebEnginePage *page;
    QMetaObject::Connection m_pageConnection;
    bool m_dragEntered;
    mutable bool m_ownsPage;
    QWebEngineContextMenuRequest *m_contextRequest;
    QScopedPointer<QtWebEngineWidgetUI::AutofillPopupWidget> m_autofillPopupWidget;
    QPointer<QtWebEngineWidgetUI::TouchSelectionMenuWidget> m_touchSelectionMenu;
};

class QContextMenuBuilder : public QtWebEngineCore::RenderViewContextMenuQt
{
public:
    QContextMenuBuilder(QWebEngineContextMenuRequest *request, QWebEngineView *view, QMenu *menu);

private:
    virtual bool hasInspector() override;
    virtual bool isFullScreenMode() override;

    virtual void addMenuItem(ContextMenuItem entry) override;
    virtual bool isMenuItemEnabled(ContextMenuItem entry) override;

    QWebEngineView *m_view;
    QMenu *m_menu;
};

QT_END_NAMESPACE

#endif // QWEBENGINEVIEW_P_H
