// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFLINKMODEL_P_H
#define QQUICKPDFLINKMODEL_P_H

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
#include <QtPdfQuick/private/qquickpdfdocument_p.h>
#include <QtPdf/private/qpdflinkmodel_p.h>

#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class Q_PDFQUICK_EXPORT QQuickPdfLinkModel : public QPdfLinkModel
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    QML_NAMED_ELEMENT(PdfLinkModel)
    QML_ADDED_IN_VERSION(5, 15)

public:
    explicit QQuickPdfLinkModel(QObject *parent = nullptr);
    ~QQuickPdfLinkModel() override;

    QQuickPdfDocument *document() const;
    void setDocument(QQuickPdfDocument *document);

private:
    QQuickPdfDocument *m_quickDocument;
    Q_DISABLE_COPY(QQuickPdfLinkModel)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfLinkModel)

#endif // QQUICKPDFLINKMODEL_P_H
