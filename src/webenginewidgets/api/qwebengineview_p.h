/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
#include <QtWidgets/qaccessiblewidget.h>

#include "render_view_context_menu_qt.h"

namespace QtWebEngineCore {
class QWebEngineContextMenuRequest;
class RenderWidgetHostViewQtDelegateWidget;
class RenderWidgetHostViewQtDelegate;
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
    void widgetChanged(QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *oldWidget,
                       QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *newWidget);

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
    QWebEngineContextMenuRequest *lastContextMenuRequest() const override;
    QWebEnginePage *createPageForWindow(QWebEnginePage::WebWindowType type) override;
    QObject *accessibilityParentObject() override;
    void didPrintPage(QPrinter *&printer, QSharedPointer<QByteArray> result) override;
    void didPrintPageToPdf(const QString &filePath, bool success) override;
    void printRequested() override;

    QWebEngineViewPrivate();
    virtual ~QWebEngineViewPrivate();
    static void bindPageAndView(QWebEnginePage *page, QWebEngineView *view);
    static void bindPageAndWidget(QWebEnginePage *page,
                                  QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *widget);
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
};

#ifndef QT_NO_ACCESSIBILITY
class QWebEngineViewAccessible : public QAccessibleWidget
{
public:
    QWebEngineViewAccessible(QWebEngineView *o) : QAccessibleWidget(o)
    {}

    bool isValid() const override;
    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;

private:
    QWebEngineView *view() const { return static_cast<QWebEngineView *>(object()); }
};
#endif // QT_NO_ACCESSIBILITY

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
