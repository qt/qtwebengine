// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "colorpicker.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

ColorPicker::ColorPicker(QWidget *parent)
    : QWidget(parent)
    , m_colorInput(new QLineEdit)
    , m_chooseButton(new QPushButton)
{
    m_chooseButton->setText(tr("Choose"));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_colorInput);
    layout->addWidget(m_chooseButton);
    setLayout(layout);

    connect(m_colorInput, &QLineEdit::textChanged, this, &ColorPicker::colorStringChanged);
    connect(m_chooseButton, &QPushButton::clicked, this, &ColorPicker::selectButtonClicked);
}

QColor ColorPicker::color() const
{
    return QColor(m_colorInput->text());
}

void ColorPicker::setColor(const QColor &color)
{
    if (color.isValid())
        m_colorInput->setText(color.name(QColor::HexArgb));
}

void ColorPicker::colorStringChanged(const QString &colorString)
{
    QColor color(colorString);
    QPalette palette;

    palette.setColor(QPalette::Base, color.isValid() ? color : QColor(Qt::white));
    m_colorInput->setPalette(palette);
}

void ColorPicker::selectButtonClicked()
{
    QColor selectedColor = QColorDialog::getColor(color().isValid() ? color() : Qt::white,
                                                  this,
                                                  "Select Color",
                                                  QColorDialog::ShowAlphaChannel);
    setColor(selectedColor);
}
