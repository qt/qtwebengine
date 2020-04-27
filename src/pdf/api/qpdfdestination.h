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

#ifndef QPDFDESTINATION_H
#define QPDFDESTINATION_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qdebug.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QPdfDestinationPrivate;

class Q_PDF_EXPORT QPdfDestination
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(int page READ page)
    Q_PROPERTY(QPointF location READ location)
    Q_PROPERTY(qreal zoom READ zoom)

public:
    ~QPdfDestination();
    QPdfDestination(const QPdfDestination &other);
    QPdfDestination &operator=(const QPdfDestination &other);
    QPdfDestination(QPdfDestination &&other) noexcept;
    QPdfDestination &operator=(QPdfDestination &&other) noexcept { swap(other); return *this; }
    void swap(QPdfDestination &other) noexcept { d.swap(other.d); }
    bool isValid() const;
    int page() const;
    QPointF location() const;
    qreal zoom() const;

protected:
    QPdfDestination();
    QPdfDestination(int page, QPointF location, qreal zoom);
    QPdfDestination(QPdfDestinationPrivate *d);
    friend class QPdfDocument;
    friend class QQuickPdfNavigationStack;

protected:
    QExplicitlySharedDataPointer<QPdfDestinationPrivate> d;
};

Q_PDF_EXPORT QDebug operator<<(QDebug, const QPdfDestination &);

QT_END_NAMESPACE

#endif // QPDFDESTINATION_H
