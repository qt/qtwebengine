// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "stylesheetdialog.h"
#include "ui_stylesheetdialog.h"

StylesheetDialog::StylesheetDialog(QWidget *parent) : QDialog(parent), ui(new Ui::StylesheetDialog)
{
    ui->setupUi(this);

    connect(ui->styleSheetList, &QListWidget::currentItemChanged, this,
            &StylesheetDialog::currentStyleSheetChanged);
    connect(ui->styleSheetList, &QListWidget::itemClicked, this,
            &StylesheetDialog::listItemClicked);
    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &StylesheetDialog::fileNameChanged);

    connect(ui->addButton, &QPushButton::clicked, this, &StylesheetDialog::addButtonClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &StylesheetDialog::removeButtonClicked);

    QSettings settings;
    settings.beginGroup("styleSheets");
    for (const auto &name : settings.allKeys()) {
        QListWidgetItem *listItem = new QListWidgetItem(name, ui->styleSheetList);
        listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
        bool checked = settings.value(name).value<StyleSheet>().second;
        listItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
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
    const QString source = settings.value(current->text()).value<StyleSheet>().first;
    ui->sourceCodeEdit->setPlainText(source);
    settings.endGroup();
}

void StylesheetDialog::listItemClicked(QListWidgetItem *item)
{
    MainWindow *window = static_cast<MainWindow *>(parent());
    const QString name = item->text();
    bool checkedStateChanged = (item->checkState() == Qt::Checked && !window->hasStyleSheet(name))
            || (item->checkState() == Qt::Unchecked && window->hasStyleSheet(name));
    if (!checkedStateChanged)
        return;

    QSettings settings;
    settings.beginGroup("styleSheets");
    const QString source = settings.value(name).value<StyleSheet>().first;

    if (item->checkState() == Qt::Checked) {
        settings.setValue(name, QVariant::fromValue(qMakePair(source, true)));
        window->insertStyleSheet(name, source, true);
    } else {
        settings.setValue(name, QVariant::fromValue(qMakePair(source, false)));
        window->removeStyleSheet(name, true);
    }

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
    const QString name = ui->fileNameEdit->text();
    const QString source = ui->sourceCodeEdit->toPlainText();
    if (name.isEmpty() || source.isEmpty())
        return;

    QListWidgetItem *listItem = new QListWidgetItem(ui->fileNameEdit->text(), ui->styleSheetList);
    listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
    listItem->setCheckState(Qt::Checked);

    MainWindow *window = static_cast<MainWindow *>(parent());
    window->insertStyleSheet(name, source, true);

    QSettings settings;
    settings.beginGroup("styleSheets");
    settings.setValue(name, QVariant::fromValue(qMakePair(source, true)));
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
