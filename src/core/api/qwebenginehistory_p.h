// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
