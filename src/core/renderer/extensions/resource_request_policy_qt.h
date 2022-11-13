// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RESOURCEREQUESTPOLICYQT_H
#define RESOURCEREQUESTPOLICYQT_H

#include <set>

#include "extensions/common/extension_id.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "ui/base/page_transition_types.h"
#include "url/origin.h"

class GURL;

namespace blink {
class WebLocalFrame;
}

namespace extensions {

class Dispatcher;
class Extension;

// Encapsulates the policy for when chrome-extension:// and
// chrome-extension-resource:// URLs can be requested.
class ResourceRequestPolicyQt
{
public:
    explicit ResourceRequestPolicyQt(Dispatcher *dispatcher);

    void OnExtensionLoaded(const Extension &extension);
    void OnExtensionUnloaded(const ExtensionId &extension);

    // Returns true if the chrome-extension:// |resource_url| can be requested
    // from |frame_url|. In some cases this decision is made based upon how
    // this request was generated. Web triggered transitions are more restrictive
    // than those triggered through UI.
    bool CanRequestResource(const GURL &resource_url,
                            blink::WebLocalFrame *frame,
                            ui::PageTransition transition_type,
                            const absl::optional<url::Origin> &initiator_origin);

private:
    Dispatcher *m_dispatcher;

    // The set of extension IDs with any potentially web- or webview-accessible
    // resources.
    std::set<ExtensionId> m_web_accessible_ids;
};
} // namespace extensions

#endif // RESOURCEREQUESTPOLICYQT_H
