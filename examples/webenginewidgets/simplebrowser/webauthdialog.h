// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBAUTHDIALOG_H
#define WEBAUTHDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QScrollArea>
#include "ui_webauthdialog.h"
#include "qwebenginewebauthuxrequest.h"

class WebAuthDialog : public QDialog
{
    Q_OBJECT
public:
    WebAuthDialog(QWebEngineWebAuthUxRequest *request, QWidget *parent = nullptr);
    ~WebAuthDialog();

    void updateDisplay();

private:
    QWebEngineWebAuthUxRequest *uxRequest;
    QButtonGroup *buttonGroup = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *selectAccountWidget = nullptr;
    QVBoxLayout *selectAccountLayout = nullptr;

    void setupSelectAccountUI();
    void setupCollectPinUI();
    void setupFinishCollectTokenUI();
    void setupErrorUI();
    void onCancelRequest();
    void onRetry();
    void onAcceptRequest();
    void clearSelectAccountButtons();

    Ui::WebAuthDialog *uiWebAuthDialog;
};

#endif // WEBAUTHDIALOG_H
