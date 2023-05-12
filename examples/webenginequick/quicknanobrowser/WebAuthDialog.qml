// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

Dialog {
    id: webAuthDialog
    anchors.centerIn: parent
    width: Math.min(browserWindow.width, browserWindow.height) / 3 * 2
    contentWidth: verticalLayout.width +10;
    contentHeight: verticalLayout.height +10;
    standardButtons: Dialog.Cancel | Dialog.Apply
    title: "WebAuth Request"

    property var selectAccount;
    property var authrequest: null;

    Connections {
        id: webauthConnection
        ignoreUnknownSignals: true
        function onStateChanged(state) {
            webAuthDialog.setupUI(state);
        }
    }

    onApplied: {
        switch (webAuthDialog.authrequest.state) {
            case WebEngineWebAuthUXRequest.CollectPIN:
                webAuthDialog.authrequest.setPin(pinEdit.text);
                break;
            case WebEngineWebAuthUXRequest.SelectAccount:
                webAuthDialog.authrequest.setSelectedAccount(webAuthDialog.selectAccount);
                break;
             default:
                 break;
        }
    }

    onRejected: {
        webAuthDialog.authrequest.cancel();
    }

    function init(request) {
        pinLabel.visible = false;
        pinEdit.visible = false;
        confirmPinLabel.visible = false;
        confirmPinEdit.visible = false;
        selectAccountModel.clear();
        webAuthDialog.authrequest = request;
        webauthConnection.target = request;
        setupUI(webAuthDialog.authrequest.state)
        webAuthDialog.visible = true;
        pinEntryError.visible = false;
    }

    function setupUI(state) {
        switch (state) {
        case WebEngineWebAuthUXRequest.SelectAccount:
            setupSelectAccountUI();
            break;
        case WebEngineWebAuthUXRequest.CollectPIN:
            setupCollectPIN();
            break;
        case WebEngineWebAuthUXRequest.FinishTokenCollection:
            setupFinishCollectToken();
            break;
        case WebEngineWebAuthUXRequest.RequestFailed:
            setupErrorUI();
            break;
        case WebEngineWebAuthUXRequest.Completed:
            webAuthDialog.close();
            break;
        }
    }

    ButtonGroup {
        id : selectAccount;
        exclusive: true;
    }

    ListModel {
        id: selectAccountModel

    }
    contentItem: Item {
        ColumnLayout  {
            id : verticalLayout
            spacing : 10

            Label {
                id: heading
                text: "";
            }

            Label {
                id: description
                text: "";
            }

            Row {
                spacing : 10
                Label {
                    id: pinLabel
                    text: "PIN";
                }
                TextInput {
                    id: pinEdit
                    text: "EnterPin"
                    enabled: true
                    focus: true
                    color: "white"
                    layer.sourceRect: {
                        Rectangle: {
                            width: 20
                            height: 20
                            color: "#00B000"
                        }
                    }
                }
            }

            Row {
                spacing : 10
                Label {
                    id: confirmPinLabel
                    text: "Confirm PIN";
                }
                TextEdit {
                    id: confirmPinEdit
                    text: ""
                }
            }

            Label {
                id: pinEntryError
                text: "";
            }

            Repeater {
                id : selectAccountRepeater
                model: selectAccountModel
                Column {
                    spacing : 5
                    RadioButton {
                        text: modelData
                        ButtonGroup.group : selectAccount;
                        onClicked: function(){
                            webAuthDialog.selectAccount = text;
                        }
                    }
                }
            }
        }
    }

    function setupSelectAccountUI() {
        webAuthDialog.selectAccount = "";
        heading.text = "Choose a passkey";
        description.text = "Which passkey do you want to use for " + webAuthDialog.authrequest.relyingPartyId;

        selectAccountModel.clear();
        var userNames = webAuthDialog.authrequest.userNames;
        for (var i = 0; i < userNames.length; i++) {
            selectAccountModel.append( {"name" : userNames[i]});
        }
        pinLabel.visible = false;
        pinEdit.visible = false;
        confirmPinLabel.visible = false;
        confirmPinEdit.visible = false;
        pinEntryError.visible = false;
        standardButton(Dialog.Apply).visible = true;
        standardButton(Dialog.Cancel).visible = true;
        standardButton(Dialog.Cancel).text ="Cancel"
    }

    function setupCollectPIN() {
        var requestInfo = webAuthDialog.authrequest.pinRequest;

        pinEdit.clear();

        if (requestInfo.reason === WebEngineWebAuthUXRequest.Challenge) {
            heading.text = "PIN required";
            description.text = "Enter the PIN for your security key";
            pinLabel.visible = true;
            pinEdit.visible = true;
            confirmPinLabel.visible = false;
            confirmPinEdit.visible = false;
        } else if (reason === WebEngineWebAuthUXRequest.Set) {
            heading.text = "Set PIN ";
            description.text = "Set new PIN for your security key";
            pinLabel.visible = true;
            pinEdit.visible = true;
            confirmPinLabel.visible = true;
            confirmPinEdit.visible = true;
        }
        pinEntryError.text = getPINErrorDetails() + " " + requestInfo.remainingAttempts + " attempts reamining";
        pinEntryError.visible = true;
        selectAccountModel.clear();
        standardButton(Dialog.Cancel).visible = true;
        standardButton(Dialog.Cancel).text ="Cancel"
        standardButton(Dialog.Apply).visible = true;
    }

    function getPINErrorDetails() {
        var requestInfo = webAuthDialog.authrequest.pinRequest;
        switch (requestInfo.error) {
        case WebEngineWebAuthUXRequest.NoError:
            return "";
        case WebEngineWebAuthUXRequest.TooShort:
            return "Too short";
        case WebEngineWebAuthUXRequest.InternalUvLocked:
            return "Internal Uv Locked";
        case WebEngineWebAuthUXRequest.WrongPIN:
            return "Wrong PIN";
        case WebEngineWebAuthUXRequest.InvalidCharacters:
            return "Invalid Characters";
        case WebEngineWebAuthUXRequest.SameAsCurrentPIN:
            return "Same as current PIN";
        }
    }

    function getRequestFailureResaon() {
        var requestFailureReason = webAuthDialog.authrequest.requestFailureReason;
        switch (requestFailureReason) {
        case WebEngineWebAuthUXRequest.Timeout:
            return " Request Timeout";
        case WebEngineWebAuthUXRequest.KeyNotRegistered:
            return "Key not registered";
        case WebEngineWebAuthUXRequest.KeyAlreadyRegistered:
            return "You already registered this device. You don't have to register it again
                    Try agin with different key or device";
        case WebEngineWebAuthUXRequest.SoftPINBlock:
            return "The security key is locked because the wrong PIN was entered too many times.
                    To unlock it, remove and reinsert it.";
        case WebEngineWebAuthUXRequest.HardPINBlock:
            return "The security key is locked because the wrong PIN was entered too many times.
                    You'll need to reset the security key.";
        case WebEngineWebAuthUXRequest.AuthenticatorRemovedDuringPINEntry:
            return "Authenticator removed during verification. Please reinsert and try again";
        case WebEngineWebAuthUXRequest.AuthenticatorMissingResidentKeys:
            return "Authenticator doesn't have resident key support";
        case WebEngineWebAuthUXRequest.AuthenticatorMissingUserVerification:
            return "Authenticator missing user verification";
        case WebEngineWebAuthUXRequest.AuthenticatorMissingLargeBlob:
            return "Authenticator missing Large Blob support";
        case WebEngineWebAuthUXRequest.NoCommonAlgorithms:
            return "No common Algorithms";
        case WebEngineWebAuthUXRequest.StorageFull:
            return "Storage full";
        case WebEngineWebAuthUXRequest.UserConsentDenied:
            return "User consent denied";
        case WebEngineWebAuthUXRequest.WinUserCancelled:
            return "User cancelled request";
        }
    }

    function setupFinishCollectToken() {
        heading.text = "Use your security key with " + webAuthDialog.authrequest.relyingPartyId;
        description.text = "Touch your security key again to complete the request.";
        pinLabel.visible = false;
        pinEdit.visible = false;
        confirmPinLabel.visible = false;
        confirmPinEdit.visible = false;
        selectAccountModel.clear();
        pinEntryError.visible = false;
        standardButton(Dialog.Apply).visible = false;
        standardButton(Dialog.Cancel).visible = true;
        standardButton(Dialog.Cancel).text ="Cancel"
    }

    function setupErrorUI() {
        heading.text = "Something went wrong";
        description.text = getRequestFailureResaon();
        pinLabel.visible = false;
        pinEdit.visible = false;
        confirmPinLabel.visible = false;
        confirmPinEdit.visible = false;
        selectAccountModel.clear();
        pinEntryError.visible = false;
        standardButton(Dialog.Apply).visible = false;
        standardButton(Dialog.Cancel).visible = true;
        standardButton(Dialog.Cancel).text ="Close"
    }
}
