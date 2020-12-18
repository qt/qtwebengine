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

#ifndef QWEBENGINEHISTORY_P_H
#define QWEBENGINEHISTORY_P_H

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
#include "qtwebenginecoreglobal_p.h"
#include "qwebenginehistory.h"
#include <QtCore/qshareddata.h>

namespace QtWebEngineCore {
class WebContentsAdapter;
class WebContentsAdapterClient;
}

QT_BEGIN_NAMESPACE

class QWebEnginePagePrivate;

class QWebEngineHistoryItemPrivate : public QSharedData
{
public:
    QWebEngineHistoryItemPrivate(QtWebEngineCore::WebContentsAdapterClient *client = nullptr, int index = 0);
    QtWebEngineCore::WebContentsAdapter *adapter() const;
    QtWebEngineCore::WebContentsAdapterClient *client;
    int index;
};

class QWebEngineHistoryModelPrivate
{
public:
    QWebEngineHistoryModelPrivate(const QWebEngineHistoryPrivate *history);
    virtual ~QWebEngineHistoryModelPrivate();

    virtual int count() const;
    virtual int index(int) const;
    virtual int offsetForIndex(int) const;

    QtWebEngineCore::WebContentsAdapter *adapter() const;
    const QWebEngineHistoryPrivate *history;
};

class QWebEngineBackHistoryModelPrivate : public QWebEngineHistoryModelPrivate
{
public:
    QWebEngineBackHistoryModelPrivate(const QWebEngineHistoryPrivate *history)
        : QWebEngineHistoryModelPrivate(history) { }

    int count() const override;
    int index(int) const override;
    int offsetForIndex(int) const override;
};

class QWebEngineForwardHistoryModelPrivate : public QWebEngineHistoryModelPrivate
{
public:
    QWebEngineForwardHistoryModelPrivate(const QWebEngineHistoryPrivate *history)
        : QWebEngineHistoryModelPrivate(history) { }

    int count() const override;
    int index(int) const override;
    int offsetForIndex(int) const override;
};

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineHistoryPrivate
{
public:
    typedef std::function<QUrl (const QUrl &)> ImageProviderUrl;
    QWebEngineHistoryPrivate(QtWebEngineCore::WebContentsAdapterClient *client,
                             const ImageProviderUrl &imageProviderUrl = ImageProviderUrl());
    ~QWebEngineHistoryPrivate();

    void updateItems() const;
    QtWebEngineCore::WebContentsAdapter *adapter() const;

    QtWebEngineCore::WebContentsAdapterClient *client;

    ImageProviderUrl imageProviderUrl;
    QUrl urlOrImageProviderUrl(const QUrl &url) const { return imageProviderUrl ? imageProviderUrl(url) : url; }

    mutable QList<QWebEngineHistoryItem> items;
    mutable QScopedPointer<QWebEngineHistoryModel> navigationModel;
    mutable QScopedPointer<QWebEngineHistoryModel> backNavigationModel;
    mutable QScopedPointer<QWebEngineHistoryModel> forwardNavigationModel;
};

QT_END_NAMESPACE

#endif // QWEBENGINEHISTORY_P_H
