// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef USER_RESOURCE_CONTROLLER_HOST_H
#define USER_RESOURCE_CONTROLLER_HOST_H

#include "qtwebenginecoreglobal_p.h"

#include <QtCore/QHash>
#include <QtCore/QScopedPointer>
#include <map>
#include "user_script.h"

namespace content {
class RenderProcessHost;
class WebContents;
class RenderFrameHost;
}

namespace mojo {
template <typename Type>
class AssociatedRemote;
}

namespace qtwebengine {
namespace mojom {
class UserResourceController;
class UserResourceControllerRenderFrame;
}
}

namespace QtWebEngineCore {

class UserScript;
using UserResourceControllerRemote = mojo::AssociatedRemote<qtwebengine::mojom::UserResourceController>;
using UserResourceControllerRenderFrameRemote = mojo::AssociatedRemote<qtwebengine::mojom::UserResourceControllerRenderFrame>;
class WebContentsAdapter;

class Q_WEBENGINECORE_PRIVATE_EXPORT UserResourceControllerHost
{

public:
    UserResourceControllerHost();
    ~UserResourceControllerHost();

    void addUserScript(const UserScript &script, WebContentsAdapter *adapter);
    bool removeUserScript(const UserScript &script, WebContentsAdapter *adapter);
    void clearAllScripts(WebContentsAdapter *adapter);
    void reserve(WebContentsAdapter *adapter, int count);

    void renderProcessStartedWithHost(content::RenderProcessHost *renderer);

private:
    Q_DISABLE_COPY(UserResourceControllerHost)
    class WebContentsObserverHelper;
    class RenderProcessObserverHelper;

    void webContentsDestroyed(content::WebContents *);
    const UserResourceControllerRenderFrameRemote &
    GetUserResourceControllerRenderFrame(content::RenderFrameHost *rfh);

    QList<UserScript> m_profileWideScripts;
    typedef QHash<content::WebContents *, QList<UserScript>> ContentsScriptsMap;
    ContentsScriptsMap m_perContentsScripts;
    QHash<content::RenderProcessHost *, UserResourceControllerRemote *> m_observedProcesses;
    QScopedPointer<RenderProcessObserverHelper> m_renderProcessObserver;
    std::map<content::RenderFrameHost *, UserResourceControllerRenderFrameRemote> m_renderFrames;
};

} // namespace

#endif // USER_RESOURCE_CONTROLLER_HOST_H
