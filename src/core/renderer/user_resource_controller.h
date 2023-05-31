// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef USER_RESOURCE_CONTROLLER_H
#define USER_RESOURCE_CONTROLLER_H

#include "content/public/renderer/render_thread_observer.h"
#include "qtwebengine/userscript/userscript.mojom.h"
#include "qtwebengine/userscript/user_script_data.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"

#include <QtCore/QHash>
#include <QtCore/QList>

namespace blink {
class WebLocalFrame;
}

namespace content {
class RenderFrame;
}

namespace QtWebEngineCore {

class UserResourceController : public content::RenderThreadObserver,
                               qtwebengine::mojom::UserResourceController
{

public:
    UserResourceController();
    void renderFrameCreated(content::RenderFrame *);
    void renderFrameDestroyed(content::RenderFrame *);
    void addScriptForFrame(const QtWebEngineCore::UserScriptData &, content::RenderFrame *);
    void removeScriptForFrame(const QtWebEngineCore::UserScriptData &, content::RenderFrame *);
    void clearScriptsForFrame(content::RenderFrame *);

    void RunScriptsAtDocumentEnd(content::RenderFrame *render_frame);
    void BindReceiver(
            mojo::PendingAssociatedReceiver<qtwebengine::mojom::UserResourceController> receiver);

private:
    Q_DISABLE_COPY(UserResourceController)

    // content::RenderThreadObserver:
    void RegisterMojoInterfaces(blink::AssociatedInterfaceRegistry *associated_interfaces) override;
    void UnregisterMojoInterfaces(blink::AssociatedInterfaceRegistry *associated_interfaces) override;

    class RenderFrameObserverHelper;

    void AddScript(const QtWebEngineCore::UserScriptData &data) override;
    void RemoveScript(const QtWebEngineCore::UserScriptData &data) override;
    void ClearScripts() override;

    void runScripts(QtWebEngineCore::UserScriptData::InjectionPoint, blink::WebLocalFrame *);

    typedef QList<uint64_t> UserScriptList;
    typedef QHash<const content::RenderFrame *, UserScriptList> FrameUserScriptMap;
    FrameUserScriptMap m_frameUserScriptMap;
    QHash<uint64_t, QtWebEngineCore::UserScriptData> m_scripts;
    mojo::AssociatedReceiver<qtwebengine::mojom::UserResourceController> m_binding;
    friend class RenderFrameObserverHelper;
};

} // namespace QtWebEngineCore
#endif // USER_RESOURCE_CONTROLLER_H
