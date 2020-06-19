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

#include "qquickwebenginefaviconprovider_p_p.h"

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"
#include "web_contents_adapter.h"

#include <QtGui/QIcon>
#include <QtGui/QPixmap>

QT_BEGIN_NAMESPACE

static inline unsigned area(const QSize &size)
{
    return size.width() * size.height();
}

static QSize largestSize(const QList<QSize> &availableSizes)
{
    QSize result;
    for (const QSize &size : availableSizes) {
        if (area(size) > area(result))
            result = size;
    }

    return result;
}

static QSize fitSize(const QList<QSize> &availableSizes, const QSize &requestedSize)
{
    Q_ASSERT(availableSizes.count());
    QSize result = largestSize(availableSizes);
    if (availableSizes.count() == 1 || area(requestedSize) >= area(result))
        return result;

    for (const QSize &size : availableSizes) {
        if (area(size) == area(requestedSize))
            return size;

        if (area(requestedSize) < area(size) && area(size) < area(result))
            result = size;
    }

    return result;
}

static QPixmap extractPixmap(const QIcon &icon, const QSize &requestedSize)
{
    Q_ASSERT(!icon.isNull());

    // If source size is not specified, use the largest icon
    if (!requestedSize.isValid())
        return icon.pixmap(largestSize(icon.availableSizes()), 1.0).copy();

    const QSize &size = fitSize(icon.availableSizes(), requestedSize);
    const QPixmap &iconPixmap = icon.pixmap(size, 1.0);
    return iconPixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).copy();
}

static QQuickWebEngineView *findViewById(const QString &id, QList<QQuickWebEngineView *> *views)
{
    for (QQuickWebEngineView *view : *views) {
        if (view->icon() == QQuickWebEngineFaviconProvider::faviconProviderUrl(QUrl(id)))
            return view;
    }

    return nullptr;
}

QString QQuickWebEngineFaviconProvider::identifier()
{
    return QStringLiteral("favicon");
}

QUrl QQuickWebEngineFaviconProvider::faviconProviderUrl(const QUrl &url)
{
    if (url.isEmpty())
        return url;

    QUrl providerUrl;
    providerUrl.setScheme(QStringLiteral("image"));
    providerUrl.setHost(identifier());
    providerUrl.setPath(
            QStringLiteral("/%1").arg(url.toString(QUrl::RemoveQuery | QUrl::RemoveFragment)));
    if (url.hasQuery())
        providerUrl.setQuery(url.query(QUrl::FullyDecoded));
    if (url.hasFragment())
        providerUrl.setFragment(url.fragment(QUrl::FullyDecoded));

    return providerUrl;
}

QQuickWebEngineFaviconProvider::QQuickWebEngineFaviconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QQuickWebEngineFaviconProvider::~QQuickWebEngineFaviconProvider() { }

QPixmap QQuickWebEngineFaviconProvider::requestPixmap(const QString &id, QSize *size,
                                                      const QSize &requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);

    if (m_views.isEmpty())
        return QPixmap();

    QQuickWebEngineView *view = findViewById(id, &m_views);
    if (!view)
        return QPixmap();

    QIcon icon = view->d_ptr->adapter->icon();
    if (icon.isNull())
        return QPixmap();

    return extractPixmap(icon, requestedSize).copy();
}

QT_END_NAMESPACE
