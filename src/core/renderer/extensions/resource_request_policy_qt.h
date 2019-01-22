/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef RESOURCEREQUESTPOLICYQT_H
#define RESOURCEREQUESTPOLICYQT_H

#include <set>

#include "base/macros.h"
#include "extensions/common/extension_id.h"
#include "ui/base/page_transition_types.h"

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
                            ui::PageTransition transition_type);

private:
    Dispatcher *m_dispatcher;

    // The set of extension IDs with any potentially web- or webview-accessible
    // resources.
    std::set<ExtensionId> m_web_accessible_ids;

    DISALLOW_COPY_AND_ASSIGN(ResourceRequestPolicyQt);
};
} // namespace extensions

#endif // RESOURCEREQUESTPOLICYQT_H
