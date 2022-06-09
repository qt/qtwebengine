// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef EXTENSIONSDISPATCHERDELEGATEQT_H
#define EXTENSIONSDISPATCHERDELEGATEQT_H

#include "extensions/renderer/dispatcher_delegate.h"

namespace QtWebEngineCore {

class ExtensionsDispatcherDelegateQt : public extensions::DispatcherDelegate
{
public:
    ExtensionsDispatcherDelegateQt();
    ~ExtensionsDispatcherDelegateQt() override;

private:
    // extensions::DispatcherDelegate implementation.
    void PopulateSourceMap(extensions::ResourceBundleSourceMap *source_map) override;
};

} // namespace QtWebEngineCore

#endif // EXTENSIONSDISPATCHERDELEGATEQT_H
