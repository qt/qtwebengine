// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ZOOMSELECTOR_H
#define ZOOMSELECTOR_H

#include <QComboBox>
#include <QPdfView>

class ZoomSelector : public QComboBox
{
    Q_OBJECT

public:
    explicit ZoomSelector(QWidget *parent = nullptr);

public slots:
    void setZoomFactor(qreal zoomFactor);

    void reset();

signals:
    void zoomModeChanged(QPdfView::ZoomMode zoomMode);
    void zoomFactorChanged(qreal zoomFactor);

private slots:
    void onCurrentTextChanged(const QString &text);
};

#endif // ZOOMSELECTOR_H
