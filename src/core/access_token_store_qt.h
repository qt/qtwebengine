// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

class AccessTokenStoreQt : public content::AccessTokenStore {
public:
    AccessTokenStoreQt();

    virtual void LoadAccessTokens(const LoadAccessTokensCallbackType& request) Q_DECL_OVERRIDE;
    virtual void SaveAccessToken(const GURL& serverUrl, const base::string16& accessToken) Q_DECL_OVERRIDE;

private:
    virtual ~AccessTokenStoreQt();
    void performWorkOnUIThread();
    void respondOnOriginatingThread(const LoadAccessTokensCallbackType& callback);


    net::URLRequestContextGetter *m_systemRequestContext;
    AccessTokenSet m_accessTokenSet;

    DISALLOW_COPY_AND_ASSIGN(AccessTokenStoreQt);
};

#endif  // ACCESS_TOKEN_STORE_QT_H
