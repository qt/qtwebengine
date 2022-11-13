// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTOFILL_POPUP_CONTROLLER_H
#define AUTOFILL_POPUP_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include <QModelIndex>
#include <QObject>
#include <QScopedPointer>
#include <QStringListModel>

namespace QtWebEngineCore {

class AutofillPopupControllerPrivate;

class Q_WEBENGINECORE_PRIVATE_EXPORT AutofillPopupController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringListModel *model READ model CONSTANT FINAL)

public:
    ~AutofillPopupController();

    QStringListModel *model() { return &m_model; }

    void selectPreviousSuggestion();
    void selectNextSuggestion();
    void selectFirstSuggestion();
    void selectLastSuggestion();

    void notifyPopupShown();
    void notifyPopupHidden();

public Q_SLOTS:
    void selectSuggestion(int index);
    void acceptSuggestion();

Q_SIGNALS:
    void currentIndexChanged(const QModelIndex &index);

private:
    AutofillPopupController(AutofillPopupControllerPrivate *);
    QScopedPointer<AutofillPopupControllerPrivate> d;

    void setCurrentIndex(const QModelIndex &index);

    // Only called by AutofillClientQt:
    void updateModel();

    QStringListModel m_model;
    QModelIndex m_currentIndex;

    friend class AutofillClientQt;
};

} // namespace QtWebEngineCore

Q_DECLARE_METATYPE(QtWebEngineCore::AutofillPopupController *)

#endif // AUTOFILL_POPUP_CONTROLLER_H
