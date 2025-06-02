// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef EXTENSIONSUIPAGEHANDLERQT_H
#define EXTENSIONSUIPAGEHANDLERQT_H

#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_ui.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "qtwebengine/browser/extensions/webui/extensions_ui_qt.mojom.h"
#include "api/qwebengineextensioninfo.h"

class ExtensionsUIPageHandlerQt : public qtwebengine::mojom::PageHandler
{
public:
    ExtensionsUIPageHandlerQt(content::WebUI *webui, Profile *profile,
                              mojo::PendingReceiver<qtwebengine::mojom::PageHandler> receiver,
                              mojo::PendingRemote<qtwebengine::mojom::Page> page);

    ExtensionsUIPageHandlerQt(const ExtensionsUIPageHandlerQt &) = delete;
    ExtensionsUIPageHandlerQt &operator=(const ExtensionsUIPageHandlerQt &) = delete;

    ~ExtensionsUIPageHandlerQt() override;

    void GetAllExtensionInfo(GetAllExtensionInfoCallback callback) override;
    void LoadExtension() override;
    void InstallExtension() override;
    void UninstallExtension(const std::string &id, UninstallExtensionCallback callback) override;
    void UnloadExtension(const std::string &id, UnloadExtensionCallback callback) override;
    void SetExtensionEnabled(const std::string &id, bool isEnabled,
                             SetExtensionEnabledCallback callback) override;

private:
    bool FindExtensionById(const std::string &id, QWebEngineExtensionInfo &extensionInfo);
    void InnerLoadExtension(const base::FilePath &path);
    void InnerInstallExtension(const base::FilePath &path);

    mojo::Receiver<qtwebengine::mojom::PageHandler> receiver_;
    mojo::Remote<qtwebengine::mojom::Page> page_;
    content::WebUI *webui_;
    Profile *profile_;
};

#endif // EXTENSIONSUIPAGEHANDLERQT_H
