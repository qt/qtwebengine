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

#ifndef QPDFSEARCHRESULT_H
#define QPDFSEARCHRESULT_H

#include <QtCore/qdebug.h>
#include <QtCore/qrect.h>
#include <QtCore/qvector.h>
#include <QtPdf/qpdfdestination.h>

QT_BEGIN_NAMESPACE

class QPdfSearchResultPrivate;

class Q_PDF_EXPORT QPdfSearchResult : public QPdfDestination
{
    Q_GADGET
    Q_PROPERTY(QString contextBefore READ contextBefore)
    Q_PROPERTY(QString contextAfter READ contextAfter)
    Q_PROPERTY(QVector<QRectF> rectangles READ rectangles)

public:
    QPdfSearchResult();
    ~QPdfSearchResult() {}

    QString contextBefore() const;
    QString contextAfter() const;
    QVector<QRectF> rectangles() const;

private:
    QPdfSearchResult(int page, QVector<QRectF> rects, QString contextBefore, QString contextAfter);
    QPdfSearchResult(QPdfSearchResultPrivate *d);
    friend class QPdfDocument;
    friend class QPdfSearchModelPrivate;
    friend class QQuickPdfNavigationStack;
};

Q_PDF_EXPORT QDebug operator<<(QDebug, const QPdfSearchResult &);

QT_END_NAMESPACE

#endif // QPDFSEARCHRESULT_H
