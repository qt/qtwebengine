// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "autofill_client_qt.h"

#include "autofill_popup_controller.h"
#include "autofill_popup_controller_p.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

#include "chrome/browser/profiles/profile.h"
#include "components/autofill/content/browser/content_autofill_driver.h"
#include "components/autofill/core/browser/browser_autofill_manager.h"
#include "components/autofill/core/common/autofill_prefs.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace QtWebEngineCore {

void AutofillClientQt::CreateForWebContents(content::WebContents *contents)
{
    DCHECK(contents);
    if (!FromWebContents(contents))
        contents->SetUserData(UserDataKey(), base::WrapUnique(new AutofillClientQt(contents)));
}

AutofillClientQt::AutofillClientQt(content::WebContents *webContents)
    : autofill::ContentAutofillClient(webContents)
    , content::WebContentsObserver(webContents)
    , m_popupController(new AutofillPopupController(new AutofillPopupControllerPrivate))
{
}

AutofillClientQt::~AutofillClientQt() { }

autofill::PersonalDataManager *AutofillClientQt::GetPersonalDataManager()
{
    return nullptr;
}

autofill::AutocompleteHistoryManager *AutofillClientQt::GetAutocompleteHistoryManager()
{
    return nullptr;
}

std::unique_ptr<autofill::AutofillManager> AutofillClientQt::CreateManager(base::PassKey<autofill::ContentAutofillDriver>, autofill::ContentAutofillDriver &driver)
{
    return base::WrapUnique(new autofill::BrowserAutofillManager(&driver, std::string()));
}

PrefService *AutofillClientQt::GetPrefs()
{
    return const_cast<PrefService *>(std::as_const(*this).GetPrefs());
}

const PrefService *AutofillClientQt::GetPrefs() const
{
    Profile *profile = Profile::FromBrowserContext(web_contents()->GetBrowserContext());
    return profile->GetPrefs();
}

void AutofillClientQt::ShowAutofillPopup(const autofill::AutofillClient::PopupOpenArgs &open_args,
                                         base::WeakPtr<autofill::AutofillPopupDelegate> delegate)
{
    m_popupController->d->delegate = delegate;
    m_popupController->d->suggestions = open_args.suggestions;
    m_popupController->updateModel();

    bool autoSelectFirstSuggestion =
        open_args.trigger_source == autofill::AutofillSuggestionTriggerSource::kTextFieldDidReceiveKeyDown;

    adapterClient()->showAutofillPopup(m_popupController.data(),
                                       QRect(toQt(gfx::ToEnclosingRect(open_args.element_bounds))),
                                       autoSelectFirstSuggestion);
}

void AutofillClientQt::UpdateAutofillPopupDataListValues(
        base::span<const autofill::SelectOption> datalist)
{
    if (datalist.empty())
        HideAutofillPopup(autofill::PopupHidingReason::kNoSuggestions);
}

void AutofillClientQt::PinPopupView()
{
    // Called by password_manager component only.
    NOTIMPLEMENTED();
}

std::vector<autofill::Suggestion> AutofillClientQt::GetPopupSuggestions() const
{
    // Called by password_manager component only.
    NOTIMPLEMENTED();
    return {};
}

void AutofillClientQt::HideAutofillPopup(autofill::PopupHidingReason)
{
    adapterClient()->hideAutofillPopup();
}

bool AutofillClientQt::IsAutocompleteEnabled() const
{
    return autofill::prefs::IsAutocompleteEnabled(GetPrefs());
}

bool AutofillClientQt::IsPasswordManagerEnabled()
{
    return false;
}

bool AutofillClientQt::IsOffTheRecord() const
{
    return web_contents()->GetBrowserContext()->IsOffTheRecord();
}

scoped_refptr<network::SharedURLLoaderFactory> AutofillClientQt::GetURLLoaderFactory()
{
    return nullptr;
}

WebContentsAdapterClient *AutofillClientQt::adapterClient()
{
    return WebContentsViewQt::from(
                   static_cast<content::WebContentsImpl *>(web_contents())->GetView())
            ->client();
}

} // namespace QtWebEngineCore
