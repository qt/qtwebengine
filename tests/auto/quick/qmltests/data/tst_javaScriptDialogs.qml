/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0
import "../mock-delegates/TestParams" 1.0

TestWebEngineView {
    id: webEngineView

    TestCase {
        id: test
        name: "WebEngineViewJavaScriptDialogs"

        function init() {
            JSDialogParams.dialogMessage = "";
            JSDialogParams.dialogTitle = "";
            JSDialogParams.dialogCount = 0;
            JSDialogParams.shouldAcceptDialog = true;
        }

        function test_alert() {
            webEngineView.url = Qt.resolvedUrl("alert.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 1)
            compare(JSDialogParams.dialogMessage, "Hello Qt")
            verify(JSDialogParams.dialogTitle.indexOf("Javascript Alert -") === 0)
        }

        function test_confirm() {
            webEngineView.url = Qt.resolvedUrl("confirm.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogMessage, "Confirm test")
            compare(JSDialogParams.dialogCount, 1)
            compare(webEngineView.title, "ACCEPTED")
            JSDialogParams.shouldAcceptDialog = false
            webEngineView.reload()
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 2)
            compare(webEngineView.title, "REJECTED")

        }

        function test_prompt() {
            JSDialogParams.inputForPrompt = "tQ olleH"
            webEngineView.url = Qt.resolvedUrl("prompt.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 1)
            compare(webEngineView.title, "tQ olleH")
            JSDialogParams.shouldAcceptDialog = false
            webEngineView.reload()
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 2)
            compare(webEngineView.title, "prompt.html")
        }
    }
}
