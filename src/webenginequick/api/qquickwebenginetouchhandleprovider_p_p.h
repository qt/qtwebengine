// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINETOUCHHANDLEPROVIDER_P_P_H
#define QQUICKWEBENGINETOUCHHANDLEPROVIDER_P_P_H

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

#include <QtQuick/qquickimageprovider.h>
#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>

QT_BEGIN_NAMESPACE

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineTouchHandleProvider : public QQuickImageProvider {
public:
    static QString identifier();
    static QUrl url(int orientation);

    QQuickWebEngineTouchHandleProvider();
    ~QQuickWebEngineTouchHandleProvider();

    void init(const QMap<int, QImage> &images);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QMap<int, QImage> m_touchHandleMap;
};


QT_END_NAMESPACE

#endif // QQUICKWEBENGINETOUCHHANDLEPROVIDER_P_P_H
