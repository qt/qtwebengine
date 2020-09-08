/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.9

import Test.Shared 1.0 as Shared

TestWebEngineView {
    id: view; width: 320; height: 320

    property bool deferError: false
    property bool acceptCertificate: false

    SignalSpy {
        id: spyError
        target: view
        signalName: 'certificateError'
    }

    TestCase {
        name: 'CertificateError'
        when: windowShown

        function initTestCase() {
            Shared.HttpsServer.setExpectError(true)
            Shared.HttpsServer.newRequest.connect(function (request) {
                request.setResponseBody('<html><body>Test</body></html>')
                request.sendResponse()
            })
            view.settings.errorPageEnabled = false
        }

        function init() {
            verify(Shared.HttpsServer.start())
        }

        function cleanup() {
            Shared.HttpsServer.stop()
            view.deferError = false
            view.acceptCertificate = false
            spyError.clear()
        }

        function test_error_data() {
            return [
                { tag: 'reject',       deferError: false, acceptCertificate: false, expectedContent: '' },
                { tag: 'defer_reject', deferError: true,  acceptCertificate: false, expectedContent: '' },
                { tag: 'defer_accept', deferError: true,  acceptCertificate: true,  expectedContent: 'Test' },
            ]
        }

        function test_error(data) {
            view.deferError = data.deferError
            view.acceptCertificate = data.acceptCertificate
            var handleCertificateError = function(error) {
                if (deferError)
                    error.defer()
                else if (acceptCertificate)
                    error.ignoreCertificateError()
                else
                    error.rejectCertificate()
            }
            view.certificateError.connect(handleCertificateError)

            view.url = Shared.HttpsServer.url()

            if (data.deferError) {
                spyError.wait()
                compare(spyError.count, 1)
                compare('', view.getBodyText())

                let error = spyError.signalArguments[0][0]
                if (data.acceptCertificate)
                    error.ignoreCertificateError()
                else
                    error.rejectCertificate()
            }

            if (data.acceptCertificate)
                verify(view.waitForLoadSucceeded())
            else
                verify(view.waitForLoadFailed())

            compare(spyError.count, 1)
            compare(data.expectedContent, view.getBodyText())

            view.certificateError.disconnect(handleCertificateError)
        }

        function test_fatalError() {
            var handleCertificateError = function(error) {
                verify(!error.overrideable);
                // QQuickWebEngineViewPrivate::allowCertificateError() will implicitly reject
                // fatal errors and it should not crash if already rejected in handler.
                error.rejectCertificate();
            }
            view.certificateError.connect(handleCertificateError);

            view.url = Qt.resolvedUrl('https://revoked.badssl.com');
            if (!view.waitForLoadFailed(10000))
                skip("Couldn't load page from network, skipping test.");
            compare(spyError.count, 1);

            view.certificateError.disconnect(handleCertificateError);
        }
    }
}
