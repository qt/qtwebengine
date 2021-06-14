/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef AUTOFILL_CLIENT_QT_H
#define AUTOFILL_CLIENT_QT_H

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "components/autofill/core/browser/autofill_client.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

#include <QScopedPointer>

namespace QtWebEngineCore {

class AutofillPopupController;
class WebContentsAdapterClient;

class AutofillClientQt : public autofill::AutofillClient,
                         public content::WebContentsUserData<AutofillClientQt>,
                         public content::WebContentsObserver
{
public:
    ~AutofillClientQt() override;

    // autofill::AutofillClient overrides:
    autofill::PersonalDataManager *GetPersonalDataManager() override;
    autofill::AutocompleteHistoryManager *GetAutocompleteHistoryManager() override;
    PrefService *GetPrefs() override;
    const PrefService *GetPrefs() const override;

    void ShowAutofillPopup(const autofill::AutofillClient::PopupOpenArgs &open_args,
                           base::WeakPtr<autofill::AutofillPopupDelegate> delegate) override;
    void UpdateAutofillPopupDataListValues(const std::vector<std::u16string> &values,
                                           const std::vector<std::u16string> &labels) override;
    void PinPopupView() override;
    autofill::AutofillClient::PopupOpenArgs GetReopenPopupArgs() const override;
    base::span<const autofill::Suggestion> GetPopupSuggestions() const override;
    void UpdatePopup(const std::vector<autofill::Suggestion> &, autofill::PopupType) override;
    void HideAutofillPopup(autofill::PopupHidingReason reason) override;
    bool IsAutocompleteEnabled() override;
    void PropagateAutofillPredictions(content::RenderFrameHost *,
                                      const std::vector<autofill::FormStructure *> &) override;

private:
    explicit AutofillClientQt(content::WebContents *webContents);

    WebContentsAdapterClient *adapterClient();

    QScopedPointer<AutofillPopupController> m_popupController;

    WEB_CONTENTS_USER_DATA_KEY_DECL();
    friend class content::WebContentsUserData<AutofillClientQt>;
};

} // namespace QtWebEngineCore

#endif // AUTOFILL_CLIENT_QT_H
