/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QPDFLINK_H
#define QPDFLINK_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qclipboard.h>

QT_BEGIN_NAMESPACE

class QPdfLinkPrivate;

class QPdfLink
{
    Q_GADGET_EXPORT(Q_PDF_EXPORT)
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(int page READ page)
    Q_PROPERTY(QPointF location READ location)
    Q_PROPERTY(qreal zoom READ zoom)
    Q_PROPERTY(QUrl url READ url)
    Q_PROPERTY(QString contextBefore READ contextBefore)
    Q_PROPERTY(QString contextAfter READ contextAfter)
    Q_PROPERTY(QList<QRectF> rectangles READ rectangles)

public:
    Q_PDF_EXPORT QPdfLink();
    Q_PDF_EXPORT ~QPdfLink();
    Q_PDF_EXPORT QPdfLink &operator=(const QPdfLink &other);

    Q_PDF_EXPORT QPdfLink(const QPdfLink &other) noexcept;
    Q_PDF_EXPORT QPdfLink(QPdfLink &&other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPdfLink)

    void swap(QPdfLink &other) noexcept { d.swap(other.d); }

    Q_PDF_EXPORT bool isValid() const;
    Q_PDF_EXPORT int page() const;
    Q_PDF_EXPORT QPointF location() const;
    Q_PDF_EXPORT qreal zoom() const;
    Q_PDF_EXPORT QUrl url() const;
    Q_PDF_EXPORT QString contextBefore() const;
    Q_PDF_EXPORT QString contextAfter() const;
    Q_PDF_EXPORT QList<QRectF> rectangles() const;
    Q_PDF_EXPORT Q_INVOKABLE QString toString() const;
    Q_PDF_EXPORT Q_INVOKABLE void copyToClipboard(QClipboard::Mode mode = QClipboard::Clipboard) const;

private: // methods
    QPdfLink(int page, QPointF location, qreal zoom);
    QPdfLink(int page, QList<QRectF> rects, QString contextBefore, QString contextAfter);
    QPdfLink(QPdfLinkPrivate *d);
    friend class QPdfDocument;
    friend class QPdfLinkModelPrivate;
    friend class QPdfSearchModelPrivate;
    friend class QPdfPageNavigator;
    friend class QQuickPdfPageNavigator;

private: // storage
    QExplicitlySharedDataPointer<QPdfLinkPrivate> d;

};
Q_DECLARE_SHARED(QPdfLink)

Q_PDF_EXPORT QDebug operator<<(QDebug, const QPdfLink &);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPdfLink)

#endif // QPDFLINK_H
