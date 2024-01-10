// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
    std::vector<autofill::Suggestion> GetPopupSuggestions() const override;
    void UpdatePopup(const std::vector<autofill::Suggestion> &, autofill::PopupType) override;
    void HideAutofillPopup(autofill::PopupHidingReason reason) override;
    bool IsAutocompleteEnabled() const override;
    bool IsPasswordManagerEnabled() override;
    void PropagateAutofillPredictions(autofill::AutofillDriver *,
                                      const std::vector<autofill::FormStructure *> &) override;
    bool IsOffTheRecord() override;
    scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() override;

private:
    explicit AutofillClientQt(content::WebContents *webContents);

    WebContentsAdapterClient *adapterClient();

    QScopedPointer<AutofillPopupController> m_popupController;

    WEB_CONTENTS_USER_DATA_KEY_DECL();
    friend class content::WebContentsUserData<AutofillClientQt>;
};

} // namespace QtWebEngineCore

#endif // AUTOFILL_CLIENT_QT_H
