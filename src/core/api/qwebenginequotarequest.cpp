// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginequotarequest.h"

#if QT_DEPRECATED_SINCE(6, 5)

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineQuotaRequest
    \since 5.11
    \inmodule QtWebEngineCore
    \deprecated [6.5] Requesting host quota is no longer supported by Chromium.

    The behavior of navigator.webkitPersistentStorage
    is identical to navigator.webkitTemporaryStorage.

    For further details, see https://crbug.com/1233525
*/

/*! \fn QWebEngineQuotaRequest::QWebEngineQuotaRequest()
    \internal
*/

void QWebEngineQuotaRequest::reject()
{
}

void QWebEngineQuotaRequest::accept()
{
}

/*!
    \property QWebEngineQuotaRequest::origin
*/

QUrl QWebEngineQuotaRequest::origin() const
{
    return QUrl();
}

/*!
    \property QWebEngineQuotaRequest::requestedSize
*/

qint64 QWebEngineQuotaRequest::requestedSize() const
{
    return 0;
}

QT_END_NAMESPACE

#endif // QT_DEPRECATED_SINCE(6, 5)

#include "moc_qwebenginequotarequest.cpp"
