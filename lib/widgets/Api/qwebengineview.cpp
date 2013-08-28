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

#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include "qwebenginepage_p.h"

#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <QStackedLayout>

void QWebEngineViewPrivate::bind(QWebEngineView *view, QWebEnginePage *page)
{
    if (view && page == view->d_func()->page)
        return;

    if (page) {
        // Un-bind page from its current view.
        if (QWebEngineView *oldView = page->d_func()->view) {
            page->disconnect(oldView);
            oldView->d_func()->page = 0;
        }
        page->d_func()->view = view;
    }

    if (view) {
        // Un-bind view from its current page.
        if (QWebEnginePage *oldPage = view->d_func()->page) {
            oldPage->disconnect(view);
            oldPage->d_func()->view = 0;
        }
        view->d_func()->page = page;
    }

    if (view && page) {
        QObject::connect(page, &QWebEnginePage::titleChanged, view, &QWebEngineView::titleChanged);
        QObject::connect(page, &QWebEnginePage::urlChanged, view, &QWebEngineView::urlChanged);
        QObject::connect(page, &QWebEnginePage::loadStarted, view, &QWebEngineView::loadStarted);
        QObject::connect(page, &QWebEnginePage::loadFinished, view, &QWebEngineView::loadFinished);
    }
}

QWebEngineViewPrivate::QWebEngineViewPrivate()
    : QWidgetPrivate(QObjectPrivateVersion)
    , page(0)
    , m_pendingContextMenuEvent(false)
{
}

QWebEngineView::QWebEngineView(QWidget *parent)
    : QWidget(*(new QWebEngineViewPrivate), parent, 0)
{
    // This causes the child RenderWidgetHostViewQtDelegateWidgets to fill this widget.
    setLayout(new QStackedLayout);
}

QWebEngineView::~QWebEngineView()
{
}

QWebEnginePage* QWebEngineView::page() const
{
    Q_D(const QWebEngineView);
    if (!d->page) {
        QWebEngineView *that = const_cast<QWebEngineView*>(this);
        that->setPage(new QWebEnginePage(that));
    }
    return d->page;
}

void QWebEngineView::setPage(QWebEnginePage* page)
{
    QWebEngineViewPrivate::bind(this, page);
}

void QWebEngineView::load(const QUrl& url)
{
    page()->load(url);
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
    Q_UNUSED(type)
    return 0;
}

bool QWebEngineView::event(QEvent *ev)
{
    Q_D(QWebEngineView);
    // We swallow spontaneous contextMenu events and synthethize those back later on when we get the
    // HandleContextMenu callback from chromium
    if (ev->type() == QEvent::ContextMenu) {
        Q_ASSERT(!d->m_pendingContextMenuEvent);
        ev->accept();
        d->m_pendingContextMenuEvent = true;
        return true;
    }
    return QWidget::event(ev);
}

void QWebEngineView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    menu->popup(event->globalPos());
}


#include "moc_qwebengineview.cpp"
