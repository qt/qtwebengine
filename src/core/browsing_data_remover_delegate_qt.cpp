// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "browsing_data_remover_delegate_qt.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/public/browser/browsing_data_remover.h"

#include <QtGlobal>

namespace QtWebEngineCore {

bool DoesOriginMatchEmbedderMask(uint64_t origin_type_mask,
                                 const url::Origin &origin,
                                 storage::SpecialStoragePolicy *policy)
{
    Q_UNUSED(origin_type_mask);
    Q_UNUSED(origin);
    Q_UNUSED(policy);
    return true;
}

content::BrowsingDataRemoverDelegate::EmbedderOriginTypeMatcher BrowsingDataRemoverDelegateQt::GetOriginTypeMatcher()
{
    return base::BindRepeating(&DoesOriginMatchEmbedderMask);
}

bool BrowsingDataRemoverDelegateQt::MayRemoveDownloadHistory()
{
    return true;
}

void BrowsingDataRemoverDelegateQt::RemoveEmbedderData(const base::Time &delete_begin,
            const base::Time &delete_end,
            uint64_t remove_mask,
            content::BrowsingDataFilterBuilder *filter_builder,
            uint64_t origin_type_mask,
            base::OnceCallback<void(/*failed_data_types=*/uint64_t)> callback)
{
    Q_UNUSED(delete_begin);
    Q_UNUSED(delete_end);
    Q_UNUSED(filter_builder);
    Q_UNUSED(origin_type_mask);

    if (remove_mask & content::BrowsingDataRemover::DATA_TYPE_CACHE)
        web_cache::WebCacheManager::GetInstance()->ClearCache();

    std::move(callback).Run(0);
}

std::vector<std::string> BrowsingDataRemoverDelegateQt::GetDomainsForDeferredCookieDeletion(uint64_t)
{
    return {};
}

} // namespace QtWebEngineCore
