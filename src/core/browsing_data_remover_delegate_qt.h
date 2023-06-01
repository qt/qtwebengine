// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BROWSING_DATA_REMOVER_DELEGATE_QT_H
#define BROWSING_DATA_REMOVER_DELEGATE_QT_H

#include <cstdint>

#include "content/public/browser/browsing_data_remover_delegate.h"

namespace QtWebEngineCore {

class BrowsingDataRemoverDelegateQt : public content::BrowsingDataRemoverDelegate {

public:
    BrowsingDataRemoverDelegateQt() {}
    ~BrowsingDataRemoverDelegateQt() override {}

    content::BrowsingDataRemoverDelegate::EmbedderOriginTypeMatcher GetOriginTypeMatcher() override;
    bool MayRemoveDownloadHistory() override;
    void RemoveEmbedderData(
        const base::Time &delete_begin,
        const base::Time &delete_end,
        uint64_t remove_mask,
        content::BrowsingDataFilterBuilder *filter_builder,
        uint64_t origin_type_mask,
        base::OnceCallback<void(/*failed_data_types=*/uint64_t)> callback) override;
    std::vector<std::string> GetDomainsForDeferredCookieDeletion(uint64_t) override;
};

} // namespace QtWebEngineCore

#endif // BROWSING_DATA_REMOVER_DELEGATE_QT_H
