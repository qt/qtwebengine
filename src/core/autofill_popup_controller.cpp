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
        d->delegate->DidSelectSuggestion(suggestion.value, suggestion.frontend_id);
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
    d->delegate->DidAcceptSuggestion(suggestion.value, suggestion.frontend_id,
                                     suggestion.backend_id, index);
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
        values.append(QString::fromStdU16String(d->suggestions[i].value));
    }
    m_model.setStringList(values);
    setCurrentIndex(QModelIndex());
}

} // namespace QtWebEngineCore
