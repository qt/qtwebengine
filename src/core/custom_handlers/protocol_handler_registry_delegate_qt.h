// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/custom_handlers/chrome_protocol_handler_registry_delegate.h:
// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROTOCOL_HANDLER_REGISTRY_DELEGATE_QT_H_
#define PROTOCOL_HANDLER_REGISTRY_DELEGATE_QT_H_

#include <string>

#include "components/custom_handlers/protocol_handler_registry.h"

namespace QtWebEngineCore {

// This class implements the ProtocolHandlerRegistry::Delegate
// abstract class to provide an OS dependent implementation
class ProtocolHandlerRegistryDelegateQt : public custom_handlers::ProtocolHandlerRegistry::Delegate {
public:
    ProtocolHandlerRegistryDelegateQt();
    ~ProtocolHandlerRegistryDelegateQt() override;

    ProtocolHandlerRegistryDelegateQt(const ProtocolHandlerRegistryDelegateQt &other) = delete;
    ProtocolHandlerRegistryDelegateQt &operator=(const ProtocolHandlerRegistryDelegateQt &other) = delete;

    // ProtocolHandlerRegistry::Delegate:
    void RegisterExternalHandler(const std::string &protocol) override;
    bool IsExternalHandlerRegistered(const std::string &protocol) override;
};

} // namespace QtWebEngineCore

#endif  // PROTOCOL_HANDLER_REGISTRY_DELEGATE_QT_H_
