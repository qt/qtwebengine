// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qrc_url_scheme_handler.h"

#include <QtWebEngineCore/qwebengineurlrequestjob.h>

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#include <memory>

namespace QtWebEngineCore {

void QrcUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    QByteArray requestMethod = job->requestMethod();
    if (requestMethod != "GET") {
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }

    QUrl requestUrl = job->requestUrl();
    QString requestPath = requestUrl.path();
    auto file = std::make_unique<QFile>(':' + requestPath, job);
    if (!file->exists() || file->size() == 0) {
        qWarning("QResource '%s' not found or is empty", qUtf8Printable(requestPath));
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }
    QFileInfo fileInfo(*file);
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileInfo);
    if (mimeType.name() == QStringLiteral("application/x-extension-html"))
        job->reply("text/html", file.release());
    else
        job->reply(mimeType.name().toUtf8(), file.release());
}

} // namespace QtWebEngineCore
