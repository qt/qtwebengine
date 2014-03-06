/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "qwebenginehistoryinterface.h"

#include "web_engine_visited_links_manager.h"
#include <QtCore/QPointer>
#include <QtCore/QUrl>


class QWebEngineHistoryInterfacePrivate : public WebEngineVisitedLinksDelegate
{
public:
    QWebEngineHistoryInterfacePrivate(QWebEngineHistoryInterface *);
    ~QWebEngineHistoryInterfacePrivate();
    virtual void addVisitedUrl(const QUrl&) Q_DECL_OVERRIDE;
    virtual QList<QUrl> visitedUrlsFromHistoryBackend() const Q_DECL_OVERRIDE;
private:
    QWebEngineHistoryInterface *q;
    friend class QWebEngineHistoryInterface;
};

static QPointer<QWebEngineHistoryInterface> g_instance;

QWebEngineHistoryInterface::QWebEngineHistoryInterface(QObject *parent)
    : QObject(parent)
    , d(new QWebEngineHistoryInterfacePrivate(this))
{
}

QWebEngineHistoryInterface::~QWebEngineHistoryInterface()
{
}

/*!
 * Sets a new default interface, \a defaultInterface, that will be used by all of QtWebEngine to keep track of visited links.
 *
 * \note The interface needs to be set before any webview is instantiated for historyContents() to be used effectively.
 */
void QWebEngineHistoryInterface::setDefaultInterface(QWebEngineHistoryInterface *defaultInterface)
{
    if (defaultInterface == g_instance.data())
        return;
    g_instance = defaultInterface;
    WebEngineVisitedLinksDelegate *delegate = defaultInterface ? defaultInterface->d.data() : 0;
    WebEngineVisitedLinksManager::instance()->setDelegate(delegate);
}

QWebEngineHistoryInterface *QWebEngineHistoryInterface::defaultInterface()
{
    return g_instance.data();
}

void QWebEngineHistoryInterface::deleteAll()
{
    WebEngineVisitedLinksManager::instance()->deleteAllVisitedLinkData();
}

void QWebEngineHistoryInterface::deleteUrls(const QList<QUrl> &urls)
{
    WebEngineVisitedLinksManager::instance()->deleteVisitedLinkDataForUrls(urls);
}

QWebEngineHistoryInterfacePrivate::QWebEngineHistoryInterfacePrivate(QWebEngineHistoryInterface *q_ptr)
    :q(q_ptr)
{
}

QWebEngineHistoryInterfacePrivate::~QWebEngineHistoryInterfacePrivate()
{
}

void QWebEngineHistoryInterfacePrivate::addVisitedUrl(const QUrl &url)
{
    q->addHistoryEntry(url);
}

QList<QUrl> QWebEngineHistoryInterfacePrivate::visitedUrlsFromHistoryBackend() const
{
    return q->historyContents();
}
