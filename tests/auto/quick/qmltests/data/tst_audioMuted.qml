// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: view
    width: 400
    height: 400

    SignalSpy {
        id: spy
        target: view
        signalName: "audioMutedChanged"
    }

    TestCase {
        id: testCase
        name: "WebEngineViewAudioMuted"

        function test_audioMuted() {
            compare(view.audioMuted, false);
            view.audioMuted = true;
            view.url = "about:blank";
            verify(view.waitForLoadSucceeded());
            compare(view.audioMuted, true);
            compare(spy.count, 1);
            compare(spy.signalArguments[0][0], true);
            view.audioMuted = false;
            compare(view.audioMuted, false);
            compare(spy.count, 2);
            compare(spy.signalArguments[1][0], false);
        }
    }
}

