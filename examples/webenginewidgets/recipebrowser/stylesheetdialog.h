// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STYLESHEETDIALOG_H
#define STYLESHEETDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class StylesheetDialog;
}
QT_END_NAMESPACE

typedef QPair<QString, bool> StyleSheet; // <source, isEnabled>
Q_DECLARE_METATYPE(StyleSheet);

class StylesheetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StylesheetDialog(QWidget *parent = 0);
    ~StylesheetDialog();

private slots:
    void currentStyleSheetChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void listItemClicked(QListWidgetItem *item);
    void fileNameChanged(const QString &text);

    void addButtonClicked();
    void removeButtonClicked();

private:
    Ui::StylesheetDialog *ui;
};

#endif // STYLESHEETDIALOG_H
