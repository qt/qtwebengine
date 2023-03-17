// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QColor>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget *parent = 0);

    QColor color() const;
    void setColor(const QColor &);

public slots:
    void colorStringChanged(const QString &);
    void selectButtonClicked();

private:
    QLineEdit *m_colorInput;
    QPushButton *m_chooseButton;
};

#endif // COLORPICKER_H
