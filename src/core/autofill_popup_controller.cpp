// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "autofill_popup_controller.h"
#include "autofill_popup_controller_p.h"

#include "components/autofill/core/browser/ui/autofill_popup_delegate.h"
#include "components/autofill/core/browser/ui/suggestion.h"

namespace QtWebEngineCore {

AutofillPopupController::AutofillPopupController(AutofillPopupControllerPrivate *dd)
{
    Q_ASSERT(dd);
    d.reset(dd);
}

AutofillPopupController::~AutofillPopupController() { }

void AutofillPopupController::setCurrentIndex(const QModelIndex &index)
{
    if (m_currentIndex == index)
        return;

    m_currentIndex = index;

    if (m_currentIndex.isValid()) {
        const autofill::Suggestion &suggestion = d->suggestions[m_currentIndex.row()];
        d->delegate->DidSelectSuggestion(suggestion.main_text.value, suggestion.frontend_id, autofill::Suggestion::BackendId());
    }

    Q_EMIT currentIndexChanged(index);
}

void AutofillPopupController::selectPreviousSuggestion()
{
    if (!m_currentIndex.isValid()) {
        setCurrentIndex(m_model.index(m_model.rowCount() - 1, 0));
        return;
    }

    if (m_currentIndex.row() == 0) {
        selectLastSuggestion();
        return;
    }

    setCurrentIndex(m_model.index(m_currentIndex.row() - 1, 0));
}

void AutofillPopupController::selectNextSuggestion()
{
    if (!m_currentIndex.isValid()) {
        setCurrentIndex(m_model.index(0, 0));
        return;
    }

    if (m_currentIndex.row() == m_model.rowCount() - 1) {
        selectFirstSuggestion();
        return;
    }

    setCurrentIndex(m_model.index(m_currentIndex.row() + 1, 0));
}

void AutofillPopupController::selectFirstSuggestion()
{
    setCurrentIndex(m_model.index(0, 0));
}

void AutofillPopupController::selectLastSuggestion()
{
    setCurrentIndex(m_model.index(m_model.rowCount() - 1, 0));
}

void AutofillPopupController::acceptSuggestion()
{
    if (!m_currentIndex.isValid())
        return;

    const int index = m_currentIndex.row();
    const autofill::Suggestion &suggestion = d->suggestions[index];
    d->delegate->DidAcceptSuggestion(suggestion, index);
}

void AutofillPopupController::notifyPopupShown()
{
    d->delegate->OnPopupShown();
}

void AutofillPopupController::notifyPopupHidden()
{
    d->delegate->OnPopupHidden();
}

void AutofillPopupController::selectSuggestion(int index)
{
    if (index < 0)
        setCurrentIndex(QModelIndex());
    else
        setCurrentIndex(m_model.index(index, 0));
}

void AutofillPopupController::updateModel()
{
    QStringList values;
    for (size_t i = 0; i < d->suggestions.size(); ++i) {
        values.append(QString::fromStdU16String(d->suggestions[i].main_text.value));
    }
    m_model.setStringList(values);
    setCurrentIndex(QModelIndex());
}

} // namespace QtWebEngineCore
