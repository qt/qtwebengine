// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "stylesheetdialog.h"
#include "ui_mainwindow.h"

#include <QWebChannel>

static QMap<QString, QString> defaultStyleSheets = {
    { "Upside down", "body { -webkit-transform: rotate(180deg); }" },
    { "Darkmode",
      "body { color: #FFF; background-color: #000 }"
      "h1, h2, h3, h4, h5 { color: #FFF }" }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_isEditMode(false)
{
    ui->setupUi(this);
    ui->textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->textEdit->hide();
    ui->webEngineView->setContextMenuPolicy(Qt::NoContextMenu);

    connect(ui->stylesheetsButton, &QPushButton::clicked, this, &MainWindow::showStyleSheetsDialog);
    connect(ui->editViewButton, &QPushButton::clicked, this, &MainWindow::toggleEditView);

    ui->recipes->insertItem(0, "Burger");
    ui->recipes->insertItem(1, "Cupcakes");
    ui->recipes->insertItem(2, "Pasta");
    ui->recipes->insertItem(3, "Pizza");
    ui->recipes->insertItem(4, "Skewers");
    ui->recipes->insertItem(5, "Soup");
    ui->recipes->insertItem(6, "Steak");
    connect(ui->recipes, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem * /* previous */) {
                const QString page = current->text().toLower();
                const QString url = QStringLiteral("qrc:/pages/") + page + QStringLiteral(".html");
                ui->webEngineView->setUrl(QUrl(url));
                m_content.setCurrentPage(page);
            });

    m_content.setTextEdit(ui->textEdit);

    connect(ui->textEdit, &QPlainTextEdit::textChanged, this,
            [this]() { m_content.setText(ui->textEdit->toPlainText()); });

    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("content"), &m_content);
    ui->webEngineView->page()->setWebChannel(channel);

    QSettings settings;
    settings.beginGroup("styleSheets");
    QStringList styleSheets = settings.allKeys();
    if (styleSheets.empty()) {
        // Add back default style sheets if the user cleared them out
        loadDefaultStyleSheets();
    } else {
        for (const auto &name : std::as_const(styleSheets)) {
            StyleSheet styleSheet = settings.value(name).value<StyleSheet>();
            if (styleSheet.second)
                insertStyleSheet(name, styleSheet.first, false);
        }
    }
    settings.endGroup();

    ui->recipes->setCurrentItem(ui->recipes->item(0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::insertStyleSheet(const QString &name, const QString &source, bool immediately)
{
    QWebEngineScript script;
    QString s = QString::fromLatin1("(function() {"
                                    "    css = document.createElement('style');"
                                    "    css.type = 'text/css';"
                                    "    css.id = '%1';"
                                    "    document.head.appendChild(css);"
                                    "    css.innerText = '%2';"
                                    "})()")
                        .arg(name, source.simplified());
    if (immediately)
        ui->webEngineView->page()->runJavaScript(s, QWebEngineScript::ApplicationWorld);

    script.setName(name);
    script.setSourceCode(s);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setRunsOnSubFrames(true);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    ui->webEngineView->page()->scripts().insert(script);
}

void MainWindow::removeStyleSheet(const QString &name, bool immediately)
{
    QString s = QString::fromLatin1("(function() {"
                                    "    var element = document.getElementById('%1');"
                                    "    element.outerHTML = '';"
                                    "    delete element;"
                                    "})()")
                        .arg(name);
    if (immediately)
        ui->webEngineView->page()->runJavaScript(s, QWebEngineScript::ApplicationWorld);

    const QList<QWebEngineScript> scripts = ui->webEngineView->page()->scripts().find(name);
    if (!scripts.isEmpty())
        ui->webEngineView->page()->scripts().remove(scripts.first());
}

bool MainWindow::hasStyleSheet(const QString &name)
{
    const QList<QWebEngineScript> scripts = ui->webEngineView->page()->scripts().find(name);
    return !scripts.isEmpty();
}

void MainWindow::loadDefaultStyleSheets()
{
    QSettings settings;
    settings.beginGroup("styleSheets");

    for (auto it = defaultStyleSheets.constBegin(); it != defaultStyleSheets.constEnd(); ++it) {
        settings.setValue(it.key(), QVariant::fromValue(qMakePair(it.value(), false)));
    }

    settings.endGroup();
}

void MainWindow::showStyleSheetsDialog()
{
    StylesheetDialog *dialog = new StylesheetDialog(this);
    dialog->show();
}

void MainWindow::toggleEditView()
{
    m_isEditMode = !m_isEditMode;

    if (m_isEditMode) {
        ui->webEngineView->hide();
        ui->textEdit->show();

        ui->editViewButton->setText(QStringLiteral("View"));
    } else {
        ui->textEdit->hide();
        ui->webEngineView->show();

        ui->editViewButton->setText(QStringLiteral("Edit"));
    }
}
