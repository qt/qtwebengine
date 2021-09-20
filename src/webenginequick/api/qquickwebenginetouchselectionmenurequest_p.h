/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
    QML_NAMED_ELEMENT(QQuickWebEngineTouchSelectionMenuRequest)
    QML_ADDED_IN_VERSION(6, 3)
    QML_UNCREATABLE("")

    QQuickWebEngineTouchSelectionMenuRequest(QRect bounds,
                                             QtWebEngineCore::TouchSelectionMenuController *touchSelectionMenuController);
    virtual ~QQuickWebEngineTouchSelectionMenuRequest();

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
