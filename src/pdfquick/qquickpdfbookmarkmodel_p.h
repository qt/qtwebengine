// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFBOOKMARKMODEL_P_H
#define QQUICKPDFBOOKMARKMODEL_P_H

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
#include <QtPdf/qpdfbookmarkmodel.h>

#include <QQmlEngine>

QT_BEGIN_NAMESPACE

class Q_PDFQUICK_EXPORT QQuickPdfBookmarkModel : public QPdfBookmarkModel
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    QML_NAMED_ELEMENT(PdfBookmarkModel)
    QML_ADDED_IN_VERSION(6, 4)

public:
    explicit QQuickPdfBookmarkModel(QObject *parent = nullptr);
    ~QQuickPdfBookmarkModel() override;

    QQuickPdfDocument *document() const;
    void setDocument(QQuickPdfDocument *document);

Q_SIGNALS:
    void documentChanged();

private:
    QQuickPdfDocument *m_quickDocument;

    Q_DISABLE_COPY(QQuickPdfBookmarkModel)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfBookmarkModel)

#endif // QQUICKPDFBOOKMARKMODEL_P_H
