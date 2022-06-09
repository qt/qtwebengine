// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINETOUCHSELECTIONMENUREQUEST_P_H
#define QQUICKWEBENGINETOUCHSELECTIONMENUREQUEST_P_H

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

#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtQml/qqmlregistration.h>

namespace QtWebEngineCore {
class TouchSelectionMenuController;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineTouchSelectionMenuRequestPrivate;

class Q_WEBENGINEQUICK_EXPORT QQuickWebEngineTouchSelectionMenuRequest : public QObject
{
    Q_OBJECT
public:
    enum TouchSelectionCommandFlag {
        Cut = 0x1,
        Copy = 0x2,
        Paste = 0x4
    };

    Q_DECLARE_FLAGS(TouchSelectionCommandFlags, TouchSelectionCommandFlag)
    Q_FLAG(TouchSelectionCommandFlags)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    Q_PROPERTY(QRect selectionBounds READ selectionBounds CONSTANT FINAL REVISION(1))
    Q_PROPERTY(TouchSelectionCommandFlags touchSelectionCommandFlags READ touchSelectionCommandFlags CONSTANT FINAL REVISION(1))
    QML_NAMED_ELEMENT(TouchSelectionMenuRequest)
    QML_ADDED_IN_VERSION(6, 3)
    QML_UNCREATABLE("")

    QQuickWebEngineTouchSelectionMenuRequest(QRect bounds,
                                             QtWebEngineCore::TouchSelectionMenuController *touchSelectionMenuController);
    ~QQuickWebEngineTouchSelectionMenuRequest();

    int buttonCount();
    bool isAccepted() const;
    void setAccepted(bool accepted);
    QRect selectionBounds();
    TouchSelectionCommandFlags touchSelectionCommandFlags() const;

private:
    QScopedPointer<QQuickWebEngineTouchSelectionMenuRequestPrivate> d;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINETOUCHSELECTIONMENUREQUEST_P_H
