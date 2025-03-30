// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qrc_url_scheme_handler.h"

#include <QtWebEngineCore/qwebengineurlrequestjob.h>

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#include <memory>

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

void QrcUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    const QByteArray requestMethod = job->requestMethod();
    if (requestMethod != "GET") {
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }

    const QUrl requestUrl = job->requestUrl();
    const QString requestPath = requestUrl.path();
    auto file = std::make_unique<QFile>(u':' + requestPath, job);
    if (!file->exists() || file->size() == 0) {
        qWarning("QResource '%s' not found or is empty", qUtf8Printable(requestPath));
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }
    QFileInfo fileInfo(*file);
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileInfo);
    if (mimeType.name() == "application/x-extension-html"_L1)
        job->reply("text/html", file.release());
    else
        job->reply(mimeType.name().toUtf8(), file.release());
}

} // namespace QtWebEngineCore
