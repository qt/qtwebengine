// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "webauthdialog.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QWebEngineView>

WebAuthDialog::WebAuthDialog(QWebEngineWebAuthUXRequest *request, QWidget *parent)
    : QDialog(parent), uxRequest(request), uiWebAuthDialog(new Ui::WebAuthDialog)
{
    uiWebAuthDialog->setupUi(this);

    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);

    scrollArea = new QScrollArea(this);
    selectAccountWidget = new QWidget(this);
    scrollArea->setWidget(selectAccountWidget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    selectAccountWidget->resize(400, 150);
    selectAccountLayout = new QVBoxLayout(selectAccountWidget);
    uiWebAuthDialog->m_mainVerticalLayout->addWidget(scrollArea);
    selectAccountLayout->setAlignment(Qt::AlignTop);

    updateDisplay();

    connect(uiWebAuthDialog->buttonBox, &QDialogButtonBox::rejected, this,
            qOverload<>(&WebAuthDialog::onCancelRequest));

    connect(uiWebAuthDialog->buttonBox, &QDialogButtonBox::accepted, this,
            qOverload<>(&WebAuthDialog::onAcceptRequest));
    QAbstractButton *button = uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry);
    connect(button, &QAbstractButton::clicked, this, qOverload<>(&WebAuthDialog::onRetry));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

WebAuthDialog::~WebAuthDialog()
{
    QList<QAbstractButton *> buttons = buttonGroup->buttons();
    auto itr = buttons.begin();
    while (itr != buttons.end()) {
        QAbstractButton *radioButton = *itr;
        delete radioButton;
        itr++;
    }

    if (buttonGroup) {
        delete buttonGroup;
        buttonGroup = nullptr;
    }

    if (uiWebAuthDialog) {
        delete uiWebAuthDialog;
        uiWebAuthDialog = nullptr;
    }

    // selectAccountWidget and it's children will get deleted when scroll area is destroyed
    if (scrollArea) {
        delete scrollArea;
        scrollArea = nullptr;
    }
}

void WebAuthDialog::updateDisplay()
{
    switch (uxRequest->state()) {
    case QWebEngineWebAuthUXRequest::SelectAccount:
        setupSelectAccountUI();
        break;
    case QWebEngineWebAuthUXRequest::CollectPIN:
        setupCollectPINUI();
        break;
    case QWebEngineWebAuthUXRequest::FinishTokenCollection:
        setupFinishCollectTokenUI();
        break;
    case QWebEngineWebAuthUXRequest::RequestFailed:
        setupErrorUI();
        break;
    default:
        break;
    }
    adjustSize();
}

void WebAuthDialog::setupSelectAccountUI()
{
    uiWebAuthDialog->m_headingLabel->setText(tr("Choose a Passkey"));
    uiWebAuthDialog->m_description->setText(tr("Which passkey do you want to use for ")
                                            + uxRequest->relyingPartyId() + tr("? "));
    uiWebAuthDialog->m_pinGroupBox->setVisible(false);
    uiWebAuthDialog->m_mainVerticalLayout->removeWidget(uiWebAuthDialog->m_pinGroupBox);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);

    clearSelectAccountButtons();
    scrollArea->setVisible(true);
    selectAccountWidget->resize(this->width(), this->height());
    QStringList userNames = uxRequest->userNames();
    // Create radio buttons for each name
    for (const QString &name : userNames) {
        QRadioButton *radioButton = new QRadioButton(name);
        selectAccountLayout->addWidget(radioButton);
        buttonGroup->addButton(radioButton);
    }

    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Ok)->setVisible(true);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);
}

void WebAuthDialog::setupFinishCollectTokenUI()
{
    clearSelectAccountButtons();
    uiWebAuthDialog->m_headingLabel->setText(tr("Use your security key with")
                                             + uxRequest->relyingPartyId());
    uiWebAuthDialog->m_description->setText(
            tr("Touch your security key again to complete the request."));
    uiWebAuthDialog->m_pinGroupBox->setVisible(false);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);
    scrollArea->setVisible(false);
}
void WebAuthDialog::setupCollectPINUI()
{
    clearSelectAccountButtons();
    uiWebAuthDialog->m_mainVerticalLayout->addWidget(uiWebAuthDialog->m_pinGroupBox);
    uiWebAuthDialog->m_pinGroupBox->setVisible(true);
    uiWebAuthDialog->m_confirmPinLabel->setVisible(false);
    uiWebAuthDialog->m_confirmPinLineEdit->setVisible(false);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Next"));
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Ok)->setVisible(true);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);
    scrollArea->setVisible(false);

    QWebEngineWebAuthPINRequest pinRequestInfo = uxRequest->pinRequest();

    if (pinRequestInfo.reason == QWebEngineWebAuthUXRequest::PINEntryReason::Challenge) {
        uiWebAuthDialog->m_headingLabel->setText(tr("PIN Required"));
        uiWebAuthDialog->m_description->setText(tr("Enter the PIN for your security key"));
        uiWebAuthDialog->m_confirmPinLabel->setVisible(false);
        uiWebAuthDialog->m_confirmPinLineEdit->setVisible(false);
    } else {
        if (pinRequestInfo.reason == QWebEngineWebAuthUXRequest::PINEntryReason::Set) {
            uiWebAuthDialog->m_headingLabel->setText(tr("New PIN Required"));
            uiWebAuthDialog->m_description->setText(tr("Set new PIN for your security key"));
        } else {
            uiWebAuthDialog->m_headingLabel->setText(tr("Change PIN Required"));
            uiWebAuthDialog->m_description->setText(tr("Change PIN for your security key"));
        }
        uiWebAuthDialog->m_confirmPinLabel->setVisible(true);
        uiWebAuthDialog->m_confirmPinLineEdit->setVisible(true);
    }

    QString errorDetails;
    switch (pinRequestInfo.error) {
    case QWebEngineWebAuthUXRequest::PINEntryError::NoError:
        break;
    case QWebEngineWebAuthUXRequest::PINEntryError::InternalUvLocked:
        errorDetails = tr("Internal User Verification Locked ");
        break;
    case QWebEngineWebAuthUXRequest::PINEntryError::WrongPIN:
        errorDetails = tr("Wrong Pin");
        break;
    case QWebEngineWebAuthUXRequest::PINEntryError::TooShort:
        errorDetails = tr("Too Short");
        break;
    case QWebEngineWebAuthUXRequest::PINEntryError::InvalidCharacters:
        errorDetails = tr("Invalid Characters");
        break;
    case QWebEngineWebAuthUXRequest::PINEntryError::SameAsCurrentPIN:
        errorDetails = tr("Same as current PIN");
        break;
    }
    if (!errorDetails.isEmpty()) {
        errorDetails += tr(" ") + QString::number(pinRequestInfo.remainingAttempts)
                + tr(" attempts remaining");
    }
    uiWebAuthDialog->m_pinEntryErrorLabel->setText(errorDetails);
}

void WebAuthDialog::onCancelRequest()
{
    uxRequest->cancel();
}

void WebAuthDialog::onAcceptRequest()
{
    switch (uxRequest->state()) {
    case QWebEngineWebAuthUXRequest::SelectAccount:
        if (buttonGroup->checkedButton()) {
            uxRequest->setSelectedAccount(buttonGroup->checkedButton()->text());
        }
        break;
    case QWebEngineWebAuthUXRequest::CollectPIN:
        uxRequest->setPin(uiWebAuthDialog->m_pinLineEdit->text());
        break;
    default:
        break;
    }
}

void WebAuthDialog::setupErrorUI()
{
    clearSelectAccountButtons();
    QString errorDescription;
    QString errorHeading = tr("Something went wrong");
    bool isVisibleRetry = false;
    switch (uxRequest->requestFailureReason()) {
    case QWebEngineWebAuthUXRequest::RequestFailureReason::Timeout:
        errorDescription = tr("Request Timeout");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::KeyNotRegistered:
        errorDescription = tr("Key not registered");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::KeyAlreadyRegistered:
        errorDescription = tr("You already registered this device."
                              "Try again with device");
        isVisibleRetry = true;
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::SoftPINBlock:
        errorDescription =
                tr("The security key is locked because the wrong PIN was entered too many times."
                   "To unlock it, remove and reinsert it.");
        isVisibleRetry = true;
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::HardPINBlock:
        errorDescription =
                tr("The security key is locked because the wrong PIN was entered too many times."
                   " You'll need to reset the security key.");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::AuthenticatorRemovedDuringPINEntry:
        errorDescription =
                tr("Authenticator removed during verification. Please reinsert and try again");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::AuthenticatorMissingResidentKeys:
        errorDescription = tr("Authenticator doesn't have resident key support");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::AuthenticatorMissingUserVerification:
        errorDescription = tr("Authenticator missing user verification");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::AuthenticatorMissingLargeBlob:
        errorDescription = tr("Authenticator missing Large Blob support");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::NoCommonAlgorithms:
        errorDescription = tr("Authenticator missing Large Blob support");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::StorageFull:
        errorDescription = tr("Storage Full");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::UserConsentDenied:
        errorDescription = tr("User consent denied");
        break;
    case QWebEngineWebAuthUXRequest::RequestFailureReason::WinUserCancelled:
        errorDescription = tr("User Cancelled Request");
        break;
    }

    uiWebAuthDialog->m_headingLabel->setText(errorHeading);
    uiWebAuthDialog->m_description->setText(errorDescription);
    uiWebAuthDialog->m_description->adjustSize();
    uiWebAuthDialog->m_pinGroupBox->setVisible(false);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry)->setVisible(isVisibleRetry);
    if (isVisibleRetry)
        uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Retry)->setFocus();
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
    uiWebAuthDialog->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Close"));
    scrollArea->setVisible(false);
}

void WebAuthDialog::onRetry()
{
    uxRequest->retry();
}

void WebAuthDialog::clearSelectAccountButtons()
{
    QList<QAbstractButton *> buttons = buttonGroup->buttons();
    auto itr = buttons.begin();
    while (itr != buttons.end()) {
        QAbstractButton *radioButton = *itr;
        selectAccountLayout->removeWidget(radioButton);
        buttonGroup->removeButton(radioButton);
        delete radioButton;
        itr++;
    }
}
