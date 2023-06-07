// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CONTENT_CLIENT_QT_H
#define CONTENT_CLIENT_QT_H

#include "qtwebenginecoreglobal_p.h"

#include "base/strings/string_piece.h"
#include "base/synchronization/lock.h"
#include "components/embedder_support/origin_trials/origin_trial_policy_impl.h"
#include "content/public/common/content_client.h"
#include "ui/base/layout.h"

#include <memory>

namespace QtWebEngineCore {

class ContentClientQt : public content::ContentClient {
public:
#if QT_CONFIG(webengine_pepper_plugins)
    void AddPlugins(std::vector<content::ContentPluginInfo> *plugins) override;
#endif
    void AddContentDecryptionModules(std::vector<content::CdmInfo> *cdms,
                                     std::vector<media::CdmHostFilePath> *cdm_host_file_paths) override;
    void AddAdditionalSchemes(Schemes* schemes) override;

    base::StringPiece GetDataResource(int, ui::ResourceScaleFactor) override;
    base::RefCountedMemory* GetDataResourceBytes(int resource_id) override;
    gfx::Image &GetNativeImageNamed(int resource_id) override;
    std::u16string GetLocalizedString(int message_id) override;
    blink::OriginTrialPolicy *GetOriginTrialPolicy() override;

private:
    // Used to lock when |origin_trial_policy_| is initialized.
    base::Lock origin_trial_policy_lock_;
    std::unique_ptr<embedder_support::OriginTrialPolicyImpl> origin_trial_policy_;
};

} // namespace QtWebEngineCore

#endif // CONTENT_CLIENT_QT_H
