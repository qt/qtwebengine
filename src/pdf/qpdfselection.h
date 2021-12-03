/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
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
