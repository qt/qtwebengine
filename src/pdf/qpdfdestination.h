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

#ifndef QPDFDESTINATION_H
#define QPDFDESTINATION_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qdebug.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QPdfDestinationPrivate;

class QPdfDestination
{
    Q_GADGET_EXPORT(Q_PDF_EXPORT)
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(int page READ page)
    Q_PROPERTY(QPointF location READ location)
    Q_PROPERTY(qreal zoom READ zoom)

public:
    Q_PDF_EXPORT ~QPdfDestination();
    Q_PDF_EXPORT QPdfDestination(const QPdfDestination &other);
    Q_PDF_EXPORT QPdfDestination &operator=(const QPdfDestination &other);

    Q_PDF_EXPORT QPdfDestination(QPdfDestination &&other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPdfDestination)

    void swap(QPdfDestination &other) noexcept { d.swap(other.d); }

    Q_PDF_EXPORT bool isValid() const;
    Q_PDF_EXPORT int page() const;
    Q_PDF_EXPORT QPointF location() const;
    Q_PDF_EXPORT qreal zoom() const;

protected:
    QPdfDestination();
    QPdfDestination(int page, QPointF location, qreal zoom);
    QPdfDestination(QPdfDestinationPrivate *d);
    friend class QPdfDocument;
    friend class QQuickPdfNavigationStack;

protected:
    QExplicitlySharedDataPointer<QPdfDestinationPrivate> d;
};
Q_DECLARE_SHARED(QPdfDestination)

Q_PDF_EXPORT QDebug operator<<(QDebug, const QPdfDestination &);

QT_END_NAMESPACE

#endif // QPDFDESTINATION_H
