// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CONTROLVIEW_H
#define CONTROLVIEW_H

#include <QInputMethodEvent>
#include <QTextCharFormat>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
QT_END_NAMESPACE

class ColorPicker;

class ControlView : public QWidget
{
    Q_OBJECT
public:
    explicit ControlView(QWidget *parent = 0);

    const QString getText() const;
    const QList<QInputMethodEvent::Attribute> getAtrributes() const;

public slots:
    void receiveInputMethodData(int, int, QTextCharFormat::UnderlineStyle, const QColor &, const QColor &, const QString &);
signals:
    void requestInputMethodEvent();

private:
    QComboBox *m_underlineStyleCombo;
    ColorPicker *m_underlineColorPicker;
    ColorPicker *m_backgroundColorPicker;
    QSpinBox *m_startSpin;
    QSpinBox *m_lengthSpin;
    QLineEdit *m_inputLine;
    QPushButton *m_sendEventButton;
};

#endif // CONTROLVIEW_H
