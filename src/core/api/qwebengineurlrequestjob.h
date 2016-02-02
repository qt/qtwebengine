/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINEURLREQUESTJOB_H
#define QWEBENGINEURLREQUESTJOB_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class URLRequestCustomJobDelegate;
class URLRequestCustomJobShared;
} // namespace

QT_BEGIN_NAMESPACE

class QIODevice;

class QWEBENGINE_EXPORT QWebEngineUrlRequestJob : public QObject {
    Q_OBJECT
public:
    ~QWebEngineUrlRequestJob();

    enum Error {
        NoError = 0,
        UrlNotFound,
        UrlInvalid,
        RequestAborted,
        RequestDenied,
        RequestFailed
    };
    Q_ENUM(Error)

    QUrl requestUrl() const;
    QByteArray requestMethod() const;

    void reply(const QByteArray &contentType, QIODevice *device);
    void fail(Error error);
    void redirect(const QUrl &url);

private:
    QWebEngineUrlRequestJob(QtWebEngineCore::URLRequestCustomJobDelegate *);
    friend class QtWebEngineCore::URLRequestCustomJobShared;

    QtWebEngineCore::URLRequestCustomJobDelegate* d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLREQUESTJOB_H
