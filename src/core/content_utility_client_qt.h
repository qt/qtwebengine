// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CONTENT_UTILITY_CLIENT_QT_H
#define CONTENT_UTILITY_CLIENT_QT_H

#include "content/public/utility/content_utility_client.h"

class MashServiceFactory;
class UtilityMessageHandler;

namespace QtWebEngineCore {

class ContentUtilityClientQt : public content::ContentUtilityClient {
public:
    ContentUtilityClientQt();
    ~ContentUtilityClientQt() override;

    // content::ContentUtilityClient:
    void RegisterIOThreadServices(mojo::ServiceFactory &services) override;
};

} // namespace

#endif
