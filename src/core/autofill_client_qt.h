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
#include "components/autofill/core/browser/autofill_manager.h"
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

    SuggestionUiSessionId
    ShowAutofillSuggestions(const PopupOpenArgs &open_args,
                            base::WeakPtr<autofill::AutofillSuggestionDelegate> delegate) override;
    void UpdateAutofillDataListValues(
            base::span<const autofill::SelectOption> datalist) override;
    void PinAutofillSuggestions() override;
    base::span<const autofill::Suggestion> GetAutofillSuggestions() const override;
    void HideAutofillSuggestions(autofill::SuggestionHidingReason reason) override;
    bool IsAutocompleteEnabled() const override;
    bool IsPasswordManagerEnabled() override;
    bool IsOffTheRecord() const override;
    scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() override;
    std::unique_ptr<autofill::AutofillManager> CreateManager(base::PassKey<autofill::ContentAutofillDriver>, autofill::ContentAutofillDriver&) override;

private:
    explicit AutofillClientQt(content::WebContents *webContents);

    WebContentsAdapterClient *adapterClient();

    QScopedPointer<AutofillPopupController> m_popupController;
};

} // namespace QtWebEngineCore

#endif // AUTOFILL_CLIENT_QT_H
