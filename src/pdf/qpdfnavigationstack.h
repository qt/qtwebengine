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

#ifndef QPDFNAVIGATIONSTACK_H
#define QPDFNAVIGATIONSTACK_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

struct QPdfNavigationStackPrivate;

class Q_PDF_EXPORT QPdfNavigationStack : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QPointF currentLocation READ currentLocation NOTIFY currentLocationChanged)
    Q_PROPERTY(qreal currentZoom READ currentZoom NOTIFY currentZoomChanged)
    Q_PROPERTY(bool backAvailable READ backAvailable NOTIFY backAvailableChanged)
    Q_PROPERTY(bool forwardAvailable READ forwardAvailable NOTIFY forwardAvailableChanged)

public:
    QPdfNavigationStack() : QPdfNavigationStack(nullptr) {}
    explicit QPdfNavigationStack(QObject *parent);
    ~QPdfNavigationStack() override;

    int currentPage() const;
    QPointF currentLocation() const;
    qreal currentZoom() const;

    bool backAvailable() const;
    bool forwardAvailable() const;

public Q_SLOTS:
    void clear();
    void jump(int page, const QPointF &location, qreal zoom);
    void update(int page, const QPointF &location, qreal zoom);
    void forward();
    void back();

Q_SIGNALS:
    void currentPageChanged(int page);
    void currentLocationChanged(QPointF location);
    void currentZoomChanged(qreal zoom);
    void backAvailableChanged(bool available);
    void forwardAvailableChanged(bool available);
    void jumped(int page, const QPointF &location, qreal zoom);

private:
    QScopedPointer<QPdfNavigationStackPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFNAVIGATIONSTACK_H
