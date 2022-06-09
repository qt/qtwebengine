// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_UI_CONTROLLER_FACTORY_QT_H_
#define WEB_UI_CONTROLLER_FACTORY_QT_H_

#include "base/memory/singleton.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_controller_factory.h"

namespace QtWebEngineCore {

class WebUIControllerFactoryQt : public content::WebUIControllerFactory
{
public:
    content::WebUI::TypeID GetWebUIType(content::BrowserContext *browserContext, const GURL &url) override;
    bool UseWebUIForURL(content::BrowserContext *browserContext, const GURL &url) override;
    std::unique_ptr<content::WebUIController> CreateWebUIControllerForURL(content::WebUI *webUi, const GURL &url) override;

    static WebUIControllerFactoryQt *GetInstance();

protected:
    WebUIControllerFactoryQt();
    ~WebUIControllerFactoryQt() override;

private:
    friend struct base::DefaultSingletonTraits<WebUIControllerFactoryQt>;
};

} // namespace QtWebEngineCore

#endif // WEB_UI_CONTROLLER_FACTORY_QT_H_
