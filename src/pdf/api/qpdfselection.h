/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
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

class Q_PDF_EXPORT QPdfSelection
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(QVector<QPolygonF> bounds READ bounds)
    Q_PROPERTY(QRectF boundingRectangle READ boundingRectangle)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(int startIndex READ startIndex)
    Q_PROPERTY(int endIndex READ endIndex)

public:
    ~QPdfSelection();
    QPdfSelection(const QPdfSelection &other);
    QPdfSelection &operator=(const QPdfSelection &other);
    QPdfSelection(QPdfSelection &&other) noexcept;
    QPdfSelection &operator=(QPdfSelection &&other) noexcept { swap(other); return *this; }
    void swap(QPdfSelection &other) noexcept { d.swap(other.d); }
    bool isValid() const;
    QVector<QPolygonF> bounds() const;
    QString text() const;
    QRectF boundingRectangle() const;
    int startIndex() const;
    int endIndex() const;
#if QT_CONFIG(clipboard)
    void copyToClipboard(QClipboard::Mode mode = QClipboard::Clipboard) const;
#endif

private:
    QPdfSelection();
    QPdfSelection(const QString &text, QVector<QPolygonF> bounds, QRectF boundingRect, int startIndex, int endIndex);
    QPdfSelection(QPdfSelectionPrivate *d);
    friend class QPdfDocument;
    friend class QQuickPdfSelection;

private:
    QExplicitlySharedDataPointer<QPdfSelectionPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFSELECTION_H
