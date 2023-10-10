// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/custom_handlers/chrome_protocol_handler_registry_delegate.cc:
// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "protocol_handler_registry_delegate_qt.h"

#include "content/public/browser/child_process_security_policy.h"
#include "url/url_util_qt.h"

namespace QtWebEngineCore {

using content::ChildProcessSecurityPolicy;

ProtocolHandlerRegistryDelegateQt::ProtocolHandlerRegistryDelegateQt() = default;

ProtocolHandlerRegistryDelegateQt::~ProtocolHandlerRegistryDelegateQt() = default;

// ProtocolHandlerRegistry::Delegate:
void ProtocolHandlerRegistryDelegateQt::RegisterExternalHandler(const std::string &protocol)
{
    ChildProcessSecurityPolicy *policy = ChildProcessSecurityPolicy::GetInstance();
    if (!policy->IsWebSafeScheme(protocol)) {
        policy->RegisterWebSafeScheme(protocol);
    }
}

bool ProtocolHandlerRegistryDelegateQt::IsExternalHandlerRegistered(const std::string &protocol)
{
    return url::IsHandledProtocol(protocol);
}

} // namespace QtWebEngineCore
