// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFPAGENAVIGATOR_P_H
#define QQUICKPDFPAGENAVIGATOR_P_H

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

#include <QtPdfQuick/private/qtpdfquickglobal_p.h>
#include <QtPdf/qpdfpagenavigator.h>
#include <QtPdf/private/qpdflink_p.h>

#include <QQmlEngine>

QT_BEGIN_NAMESPACE

struct Q_PDFQUICK_EXPORT QPdfLinkForeign
{
    Q_GADGET
    QML_FOREIGN(QPdfLink)
    QML_VALUE_TYPE(pdfLink)
    QML_ADDED_IN_VERSION(6, 4)
};

class Q_PDFQUICK_EXPORT QQuickPdfPageNavigator : public QObject
{
    Q_OBJECT
    QML_EXTENDED(QPdfPageNavigator)
    QML_NAMED_ELEMENT(PdfPageNavigator)
    QML_ADDED_IN_VERSION(5, 15)

public:
    explicit QQuickPdfPageNavigator(QObject *parent = nullptr);
    ~QQuickPdfPageNavigator() override;

private:
    QPdfPageNavigator *navStack();

    Q_DISABLE_COPY(QQuickPdfPageNavigator)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfPageNavigator)

#endif // QQUICKPDFPAGENAVIGATOR_P_H
