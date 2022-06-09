// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "controlview.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QInputMethodEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextCharFormat>
#include <QTextCursor>

#include "colorpicker.h"

ControlView::ControlView(QWidget *parent)
    : QWidget(parent)
    , m_underlineStyleCombo(new QComboBox)
    , m_underlineColorPicker(new ColorPicker)
    , m_backgroundColorPicker(new ColorPicker)
    , m_startSpin(new QSpinBox)
    , m_lengthSpin(new QSpinBox)
    , m_inputLine(new QLineEdit)
    , m_sendEventButton(new QPushButton)
{
    m_underlineStyleCombo->addItem(tr("No Underline"), QVariant(QTextCharFormat::NoUnderline));
    m_underlineStyleCombo->addItem(tr("Single Underline"), QVariant(QTextCharFormat::SingleUnderline));

    m_startSpin->setMinimum(-99);
    m_lengthSpin->setMinimum(-99);

    m_sendEventButton->setText(tr("Send Input Method Event"));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Underline Style:"), m_underlineStyleCombo);
    layout->addRow(tr("Underline Color:"), m_underlineColorPicker);
    layout->addRow(tr("Background Color:"), m_backgroundColorPicker);
    layout->addRow(tr("Start:"), m_startSpin);
    layout->addRow(tr("Length:"), m_lengthSpin);
    layout->addRow(tr("Input:"), m_inputLine);
    layout->addRow(m_sendEventButton);
    setLayout(layout);

    connect(m_sendEventButton, &QPushButton::clicked, this, &ControlView::requestInputMethodEvent);
}

void ControlView::receiveInputMethodData(int start,
                                         int length,
                                         QTextCharFormat::UnderlineStyle underlineStyle,
                                         const QColor &underlineColor,
                                         const QColor &backgroundColor,
                                         const QString &input)
{
    m_startSpin->setValue(start);
    m_lengthSpin->setValue(length);
    m_underlineStyleCombo->setCurrentIndex(m_underlineStyleCombo->findData(underlineStyle));
    m_underlineColorPicker->setColor(underlineColor);
    m_backgroundColorPicker->setColor(backgroundColor);
    m_inputLine->setText(input);
}

const QString ControlView::getText() const
{
    return m_inputLine->text();
}

const QList<QInputMethodEvent::Attribute> ControlView::getAtrributes() const
{
    int start = m_startSpin->value();
    int length = m_lengthSpin->value();

    QTextCharFormat format;
    format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(m_underlineStyleCombo->currentData().toInt()));
    format.setUnderlineColor(m_underlineColorPicker->color());

    const QColor &backgroundColor = m_backgroundColorPicker->color();
    if (backgroundColor.isValid())
        format.setBackground(QBrush(backgroundColor));

    QList<QInputMethodEvent::Attribute> attrs;
    attrs.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, start, length, format));

    return attrs;
}
