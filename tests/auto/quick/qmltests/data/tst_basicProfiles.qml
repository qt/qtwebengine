// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine
import Qt.labs.platform

Item {
    WebEngineProfile { id: otrProfile; /* MEMO implicit offTheRecord: true */ }
    WebEngineProfile { id: nonOtrProfile; offTheRecord: false; storageName: 'Test' }

    function getPath(path, offset = 1) { return path.substr(path.indexOf(':') + offset, path.length) }
    property string appDataLocation: getPath(getPath(StandardPaths.writableLocation(StandardPaths.AppDataLocation).toString(), 3))
    property string cacheLocation: getPath(getPath(StandardPaths.writableLocation(StandardPaths.CacheLocation).toString(), 3))
    property string downloadLocation: getPath(getPath(StandardPaths.writableLocation(StandardPaths.DownloadLocation).toString(), 3))

    TestCase {
        name: "BasicProfiles"

        function test_defaultProfile() {
            let p = WebEngine.defaultProfile
            verify(p.offTheRecord)

            compare(p.storageName, '')
            compare(p.cachePath, '')
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/OffTheRecord')
            compare(p.httpCacheType, WebEngineProfile.MemoryHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.NoPersistentCookies)

            compare(getPath(p.downloadPath), downloadLocation)
            compare(p.httpAcceptLanguage, '')
            verify(p.httpUserAgent !== '')
            compare(p.spellCheckEnabled, false)
            compare(p.spellCheckLanguages, [])

            compare(p.userScripts.collection, [])
        }

        function test_otrProfile() {
            let p = otrProfile
            verify(p.offTheRecord)

            compare(p.storageName, '')
            compare(p.cachePath, '')
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/OffTheRecord')
            compare(p.httpCacheType, WebEngineProfile.MemoryHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.NoPersistentCookies)

            compare(getPath(p.downloadPath), downloadLocation)
            compare(p.httpAcceptLanguage, '')
            verify(p.httpUserAgent !== '')
            compare(p.spellCheckEnabled, false)
            compare(p.spellCheckLanguages, [])

            compare(p.userScripts.collection, [])
        }

        function test_nonOtrProfile() {
            let p = nonOtrProfile
            compare(p.storageName, 'Test')
            verify(!p.offTheRecord)

            compare(getPath(p.downloadPath), downloadLocation)
            compare(p.httpAcceptLanguage, '')
            verify(p.httpUserAgent !== '')
            compare(p.spellCheckEnabled, false)
            compare(p.spellCheckLanguages, [])

            compare(p.userScripts.collection, [])

            compare(p.storageName, 'Test')
            compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)

            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
        }
    }
}
