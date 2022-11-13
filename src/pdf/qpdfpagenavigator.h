// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFPAGENAVIGATOR_H
#define QPDFPAGENAVIGATOR_H

#include <QtPdf/qtpdfglobal.h>
#include <QtPdf/qpdflink.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

struct QPdfPageNavigatorPrivate;

class Q_PDF_EXPORT QPdfPageNavigator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QPointF currentLocation READ currentLocation NOTIFY currentLocationChanged)
    Q_PROPERTY(qreal currentZoom READ currentZoom NOTIFY currentZoomChanged)
    Q_PROPERTY(bool backAvailable READ backAvailable NOTIFY backAvailableChanged)
    Q_PROPERTY(bool forwardAvailable READ forwardAvailable NOTIFY forwardAvailableChanged)

public:
    QPdfPageNavigator() : QPdfPageNavigator(nullptr) {}
    explicit QPdfPageNavigator(QObject *parent);
    ~QPdfPageNavigator() override;

    int currentPage() const;
    QPointF currentLocation() const;
    qreal currentZoom() const;

    bool backAvailable() const;
    bool forwardAvailable() const;

public Q_SLOTS:
    void clear();
    void jump(QPdfLink destination);
    void jump(int page, const QPointF &location, qreal zoom = 0);
    void update(int page, const QPointF &location, qreal zoom);
    void forward();
    void back();

Q_SIGNALS:
    void currentPageChanged(int page);
    void currentLocationChanged(QPointF location);
    void currentZoomChanged(qreal zoom);
    void backAvailableChanged(bool available);
    void forwardAvailableChanged(bool available);
    void jumped(QPdfLink current);

protected:
    QPdfLink currentLink() const;

private:
    QScopedPointer<QPdfPageNavigatorPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFPAGENAVIGATOR_H
