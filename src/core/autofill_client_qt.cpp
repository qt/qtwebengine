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

#include "autofill_client_qt.h"

#include "autofill_popup_controller.h"
#include "autofill_popup_controller_p.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

#include "base/task/thread_pool.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/browser/profiles/profile.h"
#include "components/autofill/core/common/autofill_prefs.h"
#include "content/browser/web_contents/web_contents_impl.h"

namespace QtWebEngineCore {

AutofillClientQt::AutofillClientQt(content::WebContents *webContents)
    : content::WebContentsObserver(webContents)
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

PrefService *AutofillClientQt::GetPrefs()
{
    return const_cast<PrefService *>(base::as_const(*this).GetPrefs());
}

const PrefService *AutofillClientQt::GetPrefs() const
{
    Profile *profile = Profile::FromBrowserContext(web_contents()->GetBrowserContext());
    return profile->GetPrefs();
}

void AutofillClientQt::ShowAutofillPopup(const autofill::AutofillClient::PopupOpenArgs &open_args,
                                         base::WeakPtr<autofill::AutofillPopupDelegate> delegate)
{
    // Specific popups (personal, address, credit card, password) are not supported.
    DCHECK(open_args.popup_type == autofill::PopupType::kUnspecified);

    m_popupController->d->delegate = delegate;
    m_popupController->d->suggestions = open_args.suggestions;
    m_popupController->updateModel();

    adapterClient()->showAutofillPopup(m_popupController.data(),
                                       QRect(toQt(gfx::ToEnclosingRect(open_args.element_bounds))),
                                       open_args.autoselect_first_suggestion.value());
}

void AutofillClientQt::UpdateAutofillPopupDataListValues(const std::vector<std::u16string> &values,
                                                         const std::vector<std::u16string> &labels)
{
    Q_UNUSED(labels);

    if (values.empty())
        HideAutofillPopup(autofill::PopupHidingReason::kNoSuggestions);
}

void AutofillClientQt::PinPopupView()
{
    // Called by password_manager component only.
    NOTIMPLEMENTED();
}

autofill::AutofillClient::PopupOpenArgs AutofillClientQt::GetReopenPopupArgs() const
{
    // Called by password_manager component only.
    NOTIMPLEMENTED();
    return autofill::AutofillClient::PopupOpenArgs();
}

base::span<const autofill::Suggestion> AutofillClientQt::GetPopupSuggestions() const
{
    // Called by password_manager component only.
    NOTIMPLEMENTED();
    return base::span<const autofill::Suggestion>();
}

void AutofillClientQt::UpdatePopup(const std::vector<autofill::Suggestion> &, autofill::PopupType)
{
    // Called by password_manager component only.
    NOTIMPLEMENTED();
}

void AutofillClientQt::HideAutofillPopup(autofill::PopupHidingReason)
{
    adapterClient()->hideAutofillPopup();
}

bool AutofillClientQt::IsAutocompleteEnabled()
{
    return autofill::prefs::IsAutocompleteEnabled(GetPrefs());
}

void AutofillClientQt::PropagateAutofillPredictions(content::RenderFrameHost *,
                                                    const std::vector<autofill::FormStructure *> &)
{
    // For testing purposes only.
    NOTIMPLEMENTED();
}

WebContentsAdapterClient *AutofillClientQt::adapterClient()
{
    return WebContentsViewQt::from(
                   static_cast<content::WebContentsImpl *>(web_contents())->GetView())
            ->client();
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(AutofillClientQt);

} // namespace QtWebEngineCore
