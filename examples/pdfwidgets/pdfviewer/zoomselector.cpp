// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "zoomselector.h"

#include <QLineEdit>

ZoomSelector::ZoomSelector(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);

    addItem(QLatin1String("Fit Width"));
    addItem(QLatin1String("Fit Page"));
    addItem(QLatin1String("12%"));
    addItem(QLatin1String("25%"));
    addItem(QLatin1String("33%"));
    addItem(QLatin1String("50%"));
    addItem(QLatin1String("66%"));
    addItem(QLatin1String("75%"));
    addItem(QLatin1String("100%"));
    addItem(QLatin1String("125%"));
    addItem(QLatin1String("150%"));
    addItem(QLatin1String("200%"));
    addItem(QLatin1String("400%"));

    connect(this, &QComboBox::currentTextChanged,
            this, &ZoomSelector::onCurrentTextChanged);

    connect(lineEdit(), &QLineEdit::editingFinished,
            this, [this](){onCurrentTextChanged(lineEdit()->text()); });
}

void ZoomSelector::setZoomFactor(qreal zoomFactor)
{
    setCurrentText(QString::number(qRound(zoomFactor * 100)) + QLatin1String("%"));
}

void ZoomSelector::reset()
{
    setCurrentIndex(8); // 100%
}

void ZoomSelector::onCurrentTextChanged(const QString &text)
{
    if (text == QLatin1String("Fit Width")) {
        emit zoomModeChanged(QPdfView::ZoomMode::FitToWidth);
    } else if (text == QLatin1String("Fit Page")) {
        emit zoomModeChanged(QPdfView::ZoomMode::FitInView);
    } else {
        qreal factor = 1.0;

        QString withoutPercent(text);
        withoutPercent.remove(QLatin1Char('%'));

        bool ok = false;
        const int zoomLevel = withoutPercent.toInt(&ok);
        if (ok)
            factor = zoomLevel / 100.0;

        emit zoomModeChanged(QPdfView::ZoomMode::Custom);
        emit zoomFactorChanged(factor);
    }
}
