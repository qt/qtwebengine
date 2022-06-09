// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFPAGEIMAGE_P_H
#define QQUICKPDFPAGEIMAGE_P_H

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
#include <QtQuick/private/qquickimage_p.h>

QT_BEGIN_NAMESPACE

class QQuickPdfDocument;
class QQuickPdfPageImagePrivate;
class Q_PDFQUICK_EXPORT QQuickPdfPageImage : public QQuickImage
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged FINAL)
    QML_NAMED_ELEMENT(PdfPageImage)
    QML_ADDED_IN_VERSION(6, 4)

public:
    QQuickPdfPageImage(QQuickItem *parent = nullptr);
    ~QQuickPdfPageImage() override;

    void setDocument(QQuickPdfDocument *document);
    QQuickPdfDocument *document() const;

signals:
    void documentChanged();

protected:
    void load() override;
    void documentStatusChanged();

private:
    Q_DECLARE_PRIVATE(QQuickPdfPageImage)
};

QT_END_NAMESPACE

#endif // QQUICKPDFPAGEIMAGE_P_H
