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
#include "base/containers/span.h"
#include "components/autofill/content/browser/content_autofill_client.h"
#include "content/public/browser/web_contents_observer.h"

#include <QScopedPointer>

namespace QtWebEngineCore {

class AutofillPopupController;
class WebContentsAdapterClient;

class AutofillClientQt : public autofill::ContentAutofillClient,
                         public content::WebContentsObserver
{
public:
    ~AutofillClientQt() override;

    static void CreateForWebContents(content::WebContents *contents);

    // autofill::AutofillClient overrides:
    autofill::PersonalDataManager *GetPersonalDataManager() override;
    autofill::AutocompleteHistoryManager *GetAutocompleteHistoryManager() override;
    PrefService *GetPrefs() override;
    const PrefService *GetPrefs() const override;

    void ShowAutofillPopup(const autofill::AutofillClient::PopupOpenArgs &open_args,
                           base::WeakPtr<autofill::AutofillPopupDelegate> delegate) override;
    void UpdateAutofillPopupDataListValues(
            base::span<const autofill::SelectOption> datalist) override;
    void PinPopupView() override;
    PopupOpenArgs GetReopenPopupArgs(
            autofill::AutofillSuggestionTriggerSource trigger_source) const override;
    std::vector<autofill::Suggestion> GetPopupSuggestions() const override;
    void UpdatePopup(const std::vector<autofill::Suggestion> &suggestions,
                     autofill::FillingProduct main_filling_product,
                     autofill::AutofillSuggestionTriggerSource trigger_source) override{};
    void HideAutofillPopup(autofill::PopupHidingReason reason) override;
    bool IsAutocompleteEnabled() const override;
    bool IsPasswordManagerEnabled() override;
    bool IsOffTheRecord() override;
    scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() override;

private:
    explicit AutofillClientQt(content::WebContents *webContents);

    WebContentsAdapterClient *adapterClient();

    QScopedPointer<AutofillPopupController> m_popupController;
};

} // namespace QtWebEngineCore

#endif // AUTOFILL_CLIENT_QT_H
