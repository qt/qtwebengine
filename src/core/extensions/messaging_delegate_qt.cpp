// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "messaging_delegate_qt.h"

#include <QtGlobal>

namespace extensions {

MessagingDelegateQt::MessagingDelegateQt()
{
}

absl::optional<base::Value::Dict> MessagingDelegateQt::MaybeGetTabInfo(content::WebContents *web_contents)
{
    Q_UNUSED(web_contents);
    return absl::nullopt;
}

} // namespace extensions
