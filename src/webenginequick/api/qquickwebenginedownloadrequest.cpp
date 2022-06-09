// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginedownloadrequest_p.h"
#include "QtWebEngineCore/private/qwebenginedownloadrequest_p.h"

#include "web_contents_adapter_client.h"

QT_BEGIN_NAMESPACE

/*!
    \internal
*/
QQuickWebEngineDownloadRequest::QQuickWebEngineDownloadRequest(QWebEngineDownloadRequestPrivate *p, QObject *parent)
    : QWebEngineDownloadRequest(p, parent)
{
}

/*!
    \internal
    Returns the WebEngineView the download was requested on. If the download was not triggered by content in a WebEngineView,
    \c nullptr is returned.
*/
QQuickWebEngineView *QQuickWebEngineDownloadRequest::view() const
{
    Q_ASSERT(d_ptr->adapterClient->clientType() == QtWebEngineCore::WebContentsAdapterClient::QmlClient);
    return const_cast<QQuickWebEngineView *>(static_cast<const QQuickWebEngineView *>(d_ptr->adapterClient->holdingQObject()));
}

QT_END_NAMESPACE

#include "moc_qquickwebenginedownloadrequest_p.cpp"
