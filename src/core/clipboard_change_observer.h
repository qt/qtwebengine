// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CLIPBOARD_CHANGE_OBSERVER_H
#define CLIPBOARD_CHANGE_OBSERVER_H

#include <QClipboard>
#include <QObject>

#include "ui/base/clipboard/clipboard_sequence_number_token.h"

namespace QtWebEngineCore {

class ClipboardChangeObserver : public QObject {
    Q_OBJECT
public:
    ClipboardChangeObserver();
    const ui::ClipboardSequenceNumberToken &getPrimarySequenceNumber() { return m_primarySequenceNumber; }
    const ui::ClipboardSequenceNumberToken &getSelectionSequenceNumber() { return m_selectionSequenceNumber; }

private Q_SLOTS:
    void trackChange(QClipboard::Mode mode);

private:
    ui::ClipboardSequenceNumberToken m_primarySequenceNumber;
    ui::ClipboardSequenceNumberToken m_selectionSequenceNumber;
};

} // namespace QtWebEngineCore

#endif // CLIPBOARD_CHANGE_OBSERVER_H
