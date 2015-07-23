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

#ifndef CUSTOM_PROTOCOL_HANDLER_H_
#define CUSTOM_PROTOCOL_HANDLER_H_

#include "qtwebenginecoreglobal.h"
#include "net/url_request/url_request_job_factory.h"

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE

QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace net {
class NetworkDelegate;
class URLRequestJob;
} // namespace

namespace QtWebEngineCore {

class BrowserContextAdapter;
class CustomUrlSchemeHandler;

// Implements a ProtocolHandler for custom URL schemes.
// If |network_delegate_| is NULL then all file requests will fail with ERR_ACCESS_DENIED.
class QWEBENGINE_EXPORT CustomProtocolHandler : public net::URLRequestJobFactory::ProtocolHandler {

public:
    CustomProtocolHandler(CustomUrlSchemeHandler *);

    virtual net::URLRequestJob *MaybeCreateJob(net::URLRequest *request, net::NetworkDelegate *networkDelegate) const Q_DECL_OVERRIDE;

private:
    DISALLOW_COPY_AND_ASSIGN(CustomProtocolHandler);
    CustomUrlSchemeHandler *m_schemeHandler;
};

} // namespace

#endif // CUSTOM_PROTOCOL_HANDLER_H_
