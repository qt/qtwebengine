// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTOFILL_POPUP_CONTROLLER_P_H
#define AUTOFILL_POPUP_CONTROLLER_P_H

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

#include <vector>

#include "base/memory/weak_ptr.h"
#include "components/autofill/core/browser/ui/suggestion.h"

namespace autofill {
class AutofillPopupDelegate;
}

namespace QtWebEngineCore {

class AutofillPopupControllerPrivate
{
public:
    AutofillPopupControllerPrivate() = default;

    base::WeakPtr<autofill::AutofillPopupDelegate> delegate = nullptr;
    std::vector<autofill::Suggestion> suggestions;
};

} // namespace QtWebEngineCore

#endif // AUTOFILL_POPUP_CONTROLLER_P_H
