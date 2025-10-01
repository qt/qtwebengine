// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EXTENSIONS_UI_QT_H
#define EXTENSIONS_UI_QT_H

#include "mojo/public/cpp/bindings/receiver.h"
#include "qtwebengine/browser/extensions/webui/extensions_ui_qt.mojom.h"
#include "ui/webui/mojo_web_ui_controller.h"

class ExtensionsUIPageHandlerQt;

class ExtensionsUIQt : public ui::MojoWebUIController,
                       public qtwebengine::mojom::ExtensionsUIHandlerFactory
{
public:
    explicit ExtensionsUIQt(content::WebUI *web_ui);

    ExtensionsUIQt(const ExtensionsUIQt &) = delete;
    ExtensionsUIQt &operator=(const ExtensionsUIQt &) = delete;

    void BindInterface(mojo::PendingReceiver<qtwebengine::mojom::ExtensionsUIHandlerFactory> receiver);

private:
    // qtwebengine::mojom::ExtensionsUIHandlerFactory
    void CreatePageHandler(mojo::PendingRemote<qtwebengine::mojom::Page> page,
                           mojo::PendingReceiver<qtwebengine::mojom::PageHandler> receiver) override;

    std::unique_ptr<ExtensionsUIPageHandlerQt> page_handler_;
    mojo::Receiver<qtwebengine::mojom::ExtensionsUIHandlerFactory> page_factory_receiver_ { this };
    WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif // EXTENSIONS_UI_QT_H
