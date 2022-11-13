// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEREGISTERPROTOCOLHANDLERREQUEST_H
#define QWEBENGINEREGISTERPROTOCOLHANDLERREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class RegisterProtocolHandlerRequestController;
class WebContentsDelegateQt;
} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineRegisterProtocolHandlerRequest
{
    Q_GADGET
    Q_PROPERTY(QUrl origin READ origin CONSTANT FINAL)
    Q_PROPERTY(QString scheme READ scheme CONSTANT FINAL)
public:
    QWebEngineRegisterProtocolHandlerRequest() {}
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    QUrl origin() const;
    QString scheme() const;
    bool operator==(const QWebEngineRegisterProtocolHandlerRequest &that) const { return d_ptr == that.d_ptr; }
    bool operator!=(const QWebEngineRegisterProtocolHandlerRequest &that) const { return d_ptr != that.d_ptr; }

private:
    QWebEngineRegisterProtocolHandlerRequest(QSharedPointer<QtWebEngineCore::RegisterProtocolHandlerRequestController>);
    friend QtWebEngineCore::WebContentsDelegateQt;
    QSharedPointer<QtWebEngineCore::RegisterProtocolHandlerRequestController> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEREGISTERPROTOCOLHANDLERREQUEST_H
