/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "stylesheetdialog.h"
#include "ui_stylesheetdialog.h"

StylesheetDialog::StylesheetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StylesheetDialog)
{
    ui->setupUi(this);

    connect(ui->styleSheetList, &QListWidget::currentItemChanged, this, &StylesheetDialog::currentStyleSheetChanged);
    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &StylesheetDialog::fileNameChanged);

    connect(ui->addButton, &QPushButton::clicked, this, &StylesheetDialog::addButtonClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &StylesheetDialog::removeButtonClicked);

    QSettings settings;
    settings.beginGroup("styleSheets");
    for (auto name : settings.allKeys())
        new QListWidgetItem(name,  ui->styleSheetList);
    settings.endGroup();
}

StylesheetDialog::~StylesheetDialog()
{
    delete ui;
}

void StylesheetDialog::currentStyleSheetChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!previous) {
        // Select the first item on startup
        ui->styleSheetList->setCurrentItem(current);
    }

    if (!current) {
        ui->fileNameEdit->setText(QString());
        ui->sourceCodeEdit->setPlainText(QString());
        return;
    }

    QSettings settings;
    settings.beginGroup("styleSheets");
    ui->fileNameEdit->setText(current->text());
    ui->sourceCodeEdit->setPlainText(settings.value(current->text(), QString()).toString());
    settings.endGroup();
}

void StylesheetDialog::fileNameChanged(const QString &text)
{
    QList<QListWidgetItem *> items = ui->styleSheetList->findItems(text, Qt::MatchFixedString);
    if (items.size())
        ui->addButton->setEnabled(false);
    else
        ui->addButton->setEnabled(true);
}

void StylesheetDialog::addButtonClicked()
{
    new QListWidgetItem(ui->fileNameEdit->text(),  ui->styleSheetList);

    MainWindow *window = static_cast<MainWindow *>(parent());
    const QString name = ui->fileNameEdit->text();
    const QString source = ui->sourceCodeEdit->toPlainText();
    window->insertStyleSheet(name, source, true);

    QSettings settings;
    settings.beginGroup("styleSheets");
    settings.setValue(name, source);
    settings.endGroup();

    ui->addButton->setEnabled(false);
}

void StylesheetDialog::removeButtonClicked()
{
    if (ui->styleSheetList->selectedItems().isEmpty())
        return;

    MainWindow *window = static_cast<MainWindow *>(parent());
    QSettings settings;
    settings.beginGroup("styleSheets");

    QListWidgetItem *item = ui->styleSheetList->selectedItems().first();
    const QString name = item->text();
    window->removeStyleSheet(name, true);
    settings.remove(name);
    delete item;

    settings.endGroup();
}
