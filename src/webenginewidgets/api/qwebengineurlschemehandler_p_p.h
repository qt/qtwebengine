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
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINEURLSCHEMEHANDLER_P_H
#define QWEBENGINEURLSCHEMEHANDLER_P_H

#include "qwebengineurlschemehandler_p.h"

#include "custom_url_scheme_handler.h"

#include <QPointer>

QT_BEGIN_NAMESPACE

class QWebEngineProfile;
class QWebEngineUrlRequestJob;
class QWebEngineUrlSchemeHandler;

class QWebEngineUrlSchemeHandlerPrivate : public QtWebEngineCore::CustomUrlSchemeHandler {
public:
    Q_DECLARE_PUBLIC(QWebEngineUrlSchemeHandler)

    QWebEngineUrlSchemeHandlerPrivate(const QByteArray &, QWebEngineUrlSchemeHandler *, QWebEngineProfile *);
    virtual ~QWebEngineUrlSchemeHandlerPrivate();

    virtual bool handleJob(QtWebEngineCore::URLRequestCustomJobDelegate*) Q_DECL_OVERRIDE;

private:
    QWebEngineUrlSchemeHandler *q_ptr;
    QPointer<QWebEngineProfile> m_profile;
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLSCHEMEHANDLER_P_H
