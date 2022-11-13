// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINENEWVIEWREQUEST_P_H
#define QQUICKWEBENGINENEWVIEWREQUEST_P_H

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

#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>
#include <QtWebEngineCore/qwebenginenewwindowrequest.h>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineView;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineNewWindowRequest : public QWebEngineNewWindowRequest
{
    Q_OBJECT
public:
    QML_NAMED_ELEMENT(WebEngineNewWindowRequest)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    Q_INVOKABLE void openIn(QQuickWebEngineView *);

private:
    QQuickWebEngineNewWindowRequest(DestinationType, const QRect &, const QUrl &, bool,
                                  QSharedPointer<QtWebEngineCore::WebContentsAdapter>,
                                  QObject * = nullptr);

    friend class QQuickWebEngineView;
    friend class QQuickWebEngineViewPrivate;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINENEWVIEWREQUEST_P_H
