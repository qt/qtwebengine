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

#ifndef ACCESS_TOKEN_STORE_QT_H
#define ACCESS_TOKEN_STORE_QT_H

#include "base/memory/ref_counted.h"
#include "content/public/browser/access_token_store.h"

#include <QtCore/qcompilerdetection.h>
#include <QtCore/QFile>
#include <QtCore/QScopedPointer>

namespace net {
class URLRequestContextGetter;
}

namespace QtWebEngineCore {

class AccessTokenStoreQt : public content::AccessTokenStore {
public:
    AccessTokenStoreQt();
    ~AccessTokenStoreQt();

    virtual void LoadAccessTokens(const LoadAccessTokensCallbackType& request) Q_DECL_OVERRIDE;
    virtual void SaveAccessToken(const GURL& serverUrl, const base::string16& accessToken) Q_DECL_OVERRIDE;

private:
    void performWorkOnUIThread();
    void respondOnOriginatingThread(const LoadAccessTokensCallbackType& callback);


    net::URLRequestContextGetter *m_systemRequestContext;
    AccessTokenSet m_accessTokenSet;

    DISALLOW_COPY_AND_ASSIGN(AccessTokenStoreQt);
};

} // namespace QtWebEngineCore

#endif  // ACCESS_TOKEN_STORE_QT_H
