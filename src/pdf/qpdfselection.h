// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFSELECTION_H
#define QPDFSELECTION_H

#include <QtPdf/qtpdfglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qclipboard.h>
#include <QtGui/qpolygon.h>

QT_BEGIN_NAMESPACE

class QPdfSelectionPrivate;

class QPdfSelection
{
    Q_GADGET_EXPORT(Q_PDF_EXPORT)
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(QList<QPolygonF> bounds READ bounds)
    Q_PROPERTY(QRectF boundingRectangle READ boundingRectangle)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(int startIndex READ startIndex)
    Q_PROPERTY(int endIndex READ endIndex)

public:
    Q_PDF_EXPORT ~QPdfSelection();
    Q_PDF_EXPORT QPdfSelection(const QPdfSelection &other);
    Q_PDF_EXPORT QPdfSelection &operator=(const QPdfSelection &other);

    Q_PDF_EXPORT QPdfSelection(QPdfSelection &&other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPdfSelection)

    void swap(QPdfSelection &other) noexcept { d.swap(other.d); }

    Q_PDF_EXPORT bool isValid() const;
    Q_PDF_EXPORT QList<QPolygonF> bounds() const;
    Q_PDF_EXPORT QString text() const;
    Q_PDF_EXPORT QRectF boundingRectangle() const;
    Q_PDF_EXPORT int startIndex() const;
    Q_PDF_EXPORT int endIndex() const;
#if QT_CONFIG(clipboard)
    Q_PDF_EXPORT void copyToClipboard(QClipboard::Mode mode = QClipboard::Clipboard) const;
#endif

private:
    QPdfSelection();
    QPdfSelection(const QString &text, QList<QPolygonF> bounds, QRectF boundingRect, int startIndex, int endIndex);
    QPdfSelection(QPdfSelectionPrivate *d);
    friend class QPdfDocument;
    friend class QQuickPdfSelection;

private:
    QExplicitlySharedDataPointer<QPdfSelectionPrivate> d;
};
Q_DECLARE_SHARED(QPdfSelection)

QT_END_NAMESPACE

#endif // QPDFSELECTION_H
