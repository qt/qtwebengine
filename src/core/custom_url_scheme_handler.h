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

#ifndef CUSTOM_URL_SCHEME_HANDLER_H_
#define CUSTOM_URL_SCHEME_HANDLER_H_

#include "qtwebenginecoreglobal.h"

#include <QtCore/QByteArray>
#include <QtCore/QScopedPointer>

QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace QtWebEngineCore {

class BrowserContextAdapter;
class CustomProtocolHandler;
class URLRequestCustomJobDelegate;

class QWEBENGINE_EXPORT CustomUrlSchemeHandler {
public:
    explicit CustomUrlSchemeHandler(const QByteArray &);
    ~CustomUrlSchemeHandler();

    QByteArray scheme() const;
    void setScheme(const QByteArray &);
    CustomProtocolHandler *createProtocolHandler();

    virtual bool handleJob(URLRequestCustomJobDelegate*) = 0;

private:
    QByteArray m_scheme;
};


} // namespace

#endif // CUSTOM_URL_SCHEME_HANDLER_H_
