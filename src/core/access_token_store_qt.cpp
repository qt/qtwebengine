// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "access_token_store_qt.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"

#include "browser_context_qt.h"
#include "browser_context_adapter.h"
#include "content_browser_client_qt.h"
#include "web_engine_context.h"

using content::AccessTokenStore;
using content::BrowserThread;

AccessTokenStoreQt::AccessTokenStoreQt()
    : m_systemRequestContext(0)
{
}

AccessTokenStoreQt::~AccessTokenStoreQt()
{
}

void AccessTokenStoreQt::LoadAccessTokens(const LoadAccessTokensCallbackType& callback)
{
    BrowserThread::PostTaskAndReply(BrowserThread::UI, FROM_HERE
                , base::Bind(&AccessTokenStoreQt::performWorkOnUIThread, this)
                , base::Bind(&AccessTokenStoreQt::respondOnOriginatingThread, this, callback));
}

void AccessTokenStoreQt::performWorkOnUIThread()
{
    m_systemRequestContext = WebEngineContext::current()->defaultBrowserContext()->browserContext()->GetRequestContext();
}

void AccessTokenStoreQt::respondOnOriginatingThread(const LoadAccessTokensCallbackType& callback)
{
    callback.Run(m_accessTokenSet, m_systemRequestContext);
    m_systemRequestContext = 0;
}

void AccessTokenStoreQt::SaveAccessToken(const GURL& serverUrl, const base::string16& accessToken)
{
    m_accessTokenSet[serverUrl] = accessToken;
}
