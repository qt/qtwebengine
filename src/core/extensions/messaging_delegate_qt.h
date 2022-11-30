// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef MESSAGING_DELEGATE_QT_H
#define MESSAGING_DELEGATE_QT_H

#include "extensions/browser/api/messaging/messaging_delegate.h"

namespace base {
class DictionaryValue;
}

namespace content {
class WebContents;
}

namespace extensions {

class MessagingDelegateQt : public MessagingDelegate
{
public:
    MessagingDelegateQt();

    // MessagingDelegate implementation.
    absl::optional<base::Value::Dict> MaybeGetTabInfo(content::WebContents *web_contents) override;
};

} // namespace extensions

#endif // MESSAGING_DELEGATE_QT_H
