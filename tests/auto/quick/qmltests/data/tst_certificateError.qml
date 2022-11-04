// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

import Test.Shared as Shared

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
        id: testCase
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
                    error.acceptCertificate()
                else
                    error.rejectCertificate()
            }
            view.certificateError.connect(handleCertificateError)

            const server_url = Shared.HttpsServer.url()
            view.url = server_url

            if (data.deferError) {
                spyError.wait()
                compare(spyError.count, 1)
                compare('', view.getBodyText())

                let error = spyError.signalArguments[0][0]
                if (data.acceptCertificate)
                    error.acceptCertificate()
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

            let error = spyError.signalArguments[0][0]
            compare(error.url, server_url)
            verify(error.description.length > 0)
            verify(error.overridable)
            compare(error.type, WebEngineCertificateError.CertificateAuthorityInvalid)
        }

        function test_fatalError() {
            let error = undefined
            var handleCertificateError = function(e) { error = e; }
            view.certificateError.connect(handleCertificateError);

            view.url = Qt.resolvedUrl('https://revoked.badssl.com');
            if (!view.waitForLoadResult()) {
                verify(!error, "There shouldn't be any certificate error if not loaded due to missing internet access!");
                skip("Couldn't load page from network, skipping test.");
            }
            view.certificateError.disconnect(handleCertificateError);

            // revoked certificate might not be reported as invalid by chromium and the load will silently succeed
            const failed = view.loadStatus == WebEngineView.LoadFailedStatus, hasError = Boolean(error)
            compare(hasError, failed)
            if (failed) {
                verify(!error.overridable);
                // Fatal certificate errors are implicitly rejected. But second call should not cause crash.
                error.rejectCertificate();
            }
        }
    }
}
