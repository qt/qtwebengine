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

#include "qwebenginepage.h"
#include "qwebenginepage_p.h"

#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"
#include "qwebengineview.h"
#include "qwebengineview_p.h"
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"

#include <QUrl>
#include <QLayout>

QWebEnginePagePrivate::QWebEnginePagePrivate()
    : QObjectPrivate(QObjectPrivateVersion)
    , adapter(new WebContentsAdapter(this))
    , history(new QWebEngineHistory)
    , view(0)
    , m_isLoading(false)
{
    history->d_func()->pagePrivate = this;
}

QWebEnginePagePrivate::~QWebEnginePagePrivate()
{
    delete history;
}

void QWebEnginePagePrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->titleChanged(title);
}

void QWebEnginePagePrivate::urlChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->urlChanged(url);
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
}

QRectF QWebEnginePagePrivate::viewportRect() const
{
    return view ? view->geometry() : QRectF();
}

void QWebEnginePagePrivate::loadFinished(bool success)
{
    Q_Q(QWebEnginePage);
    m_isLoading = adapter->isLoading();
    Q_EMIT q->loadFinished(success);
}

void QWebEnginePagePrivate::focusContainer()
{
    if (view)
        view->setFocus();
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegate()
{
    return new RenderWidgetHostViewQtDelegateWidget;
}

QWebEnginePage::QWebEnginePage(QObject* parent)
    : QObject(*new QWebEnginePagePrivate, parent)
{
}

QWebEnginePage::~QWebEnginePage()
{
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

void QWebEnginePage::triggerAction(WebAction action, bool)
{
    Q_D(QWebEnginePage);
    switch (action) {
    case Back:
        d->adapter->navigateHistory(-1);
        break;
    case Forward:
        d->adapter->navigateHistory(1);
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

void QWebEnginePage::load(const QUrl& url)
{
    Q_D(QWebEnginePage);
    d->adapter->load(url);
}

#include "moc_qwebenginepage.cpp"
