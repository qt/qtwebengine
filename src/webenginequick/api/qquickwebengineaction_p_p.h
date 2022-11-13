// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEACTION_P_P_H
#define QQUICKWEBENGINEACTION_P_P_H

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

#include "qtwebenginequickglobal_p.h"
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineAction;

class QQuickWebEngineActionPrivate
{
public:
    Q_DECLARE_PUBLIC(QQuickWebEngineAction)
    QQuickWebEngineActionPrivate(const QVariant &data, const QString &text, const QString &iconName, bool enabled);
    ~QQuickWebEngineActionPrivate();

    void setEnabled(bool enabled);

    QVariant data() const;

    void trigger();

    QVariant m_data;
    QString m_text;
    QString m_iconName;
    bool m_enabled;

private:
    QQuickWebEngineAction *q_ptr;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEACTION_P_P_H
