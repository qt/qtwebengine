// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>

#include "controlview.h"
#include "referenceview.h"
#include "testview.h"
#include "webview.h"

// Test for input method events for QWebEngineView. This makes possible to
// manually customize QInputMethodEvent and validate that it is rendered
// properly in a web view.

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private slots:
    void sendInputMethodEvents();

private:
    void closeEvent(QCloseEvent *event) override;

    ControlView *m_controlView;
    ReferenceView *m_referenceView;
    WebView *m_webView;
    TestView *m_testView;

    QLabel *m_referenceProcessed;
    QLabel *m_webProcessed;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_controlView(new ControlView)
    , m_referenceView(new ReferenceView)
    , m_webView(new WebView)
    , m_testView(new TestView)
    , m_referenceProcessed(new QLabel)
    , m_webProcessed(new QLabel)
{
    m_controlView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_controlView->setFixedWidth(300);

    m_webView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_webView->setFixedWidth(280);
    m_webView->setFixedHeight(150);

    m_testView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_testView->setMinimumWidth(400);

    QWidget *centralWidget = new QWidget;

    QGroupBox *referenceGroup = new QGroupBox(tr("Reference"));
    referenceGroup->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    referenceGroup->setMinimumWidth(300);
    QFormLayout *referenceProcessedForm = new QFormLayout;
    referenceProcessedForm->addRow(tr("Processed:"), m_referenceProcessed);
    QVBoxLayout *referenceLayout = new QVBoxLayout;
    referenceLayout->addWidget(m_referenceView);
    referenceLayout->addLayout(referenceProcessedForm);
    referenceGroup->setLayout(referenceLayout);

    QGroupBox *webGroup = new QGroupBox(tr("Web"));
    webGroup->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    webGroup->setMinimumWidth(300);
    QFormLayout *webProcessedForm = new QFormLayout;
    webProcessedForm->addRow(tr("Processed:"), m_webProcessed);
    QVBoxLayout *webLayout = new QVBoxLayout;
    webLayout->addWidget(m_webView);
    webLayout->addLayout(webProcessedForm);
    webGroup->setLayout(webLayout);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(m_controlView);
    leftLayout->addWidget(referenceGroup);
    leftLayout->addWidget(webGroup);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addLayout(leftLayout);
    centralLayout->addWidget(m_testView);

    connect(m_testView, &TestView::sendInputMethodData, m_controlView,
            &ControlView::receiveInputMethodData);
    connect(m_testView, &TestView::requestInputMethodEvent, this,
            &MainWindow::sendInputMethodEvents);
    connect(m_controlView, &ControlView::requestInputMethodEvent, this,
            &MainWindow::sendInputMethodEvents);

    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);
    setWindowTitle(tr("Input Methods Format Manual Test"));
}

void MainWindow::sendInputMethodEvents()
{
    bool processed;
    QString resultText;

    QString text = m_controlView->getText();
    QList<QInputMethodEvent::Attribute> attrs = m_controlView->getAtrributes();
    QInputMethodEvent im(text, attrs);

    processed = QApplication::sendEvent(m_referenceView->referenceInput(), &im);
    resultText = processed ? QStringLiteral("<font color='green'>TRUE</font>")
                           : QStringLiteral("<font color='red'>FALSE</font>");
    m_referenceProcessed->setText(resultText);

    processed = QApplication::sendEvent(m_webView->focusProxy(), &im);
    resultText = processed ? QStringLiteral("<font color='green'>TRUE</font>")
                           : QStringLiteral("<font color='red'>FALSE</font>");
    m_webProcessed->setText(resultText);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_testView->cancelTest();
    QMainWindow::closeEvent(event);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

#include "main.moc"
