/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

import QtQuick
import QtTest
import QtWebEngine
import Qt.labs.platform

Item {
    WebEngineProfile { id: otrProfile; /* MEMO implicit offTheRecord: true */ }
    WebEngineProfile { id: nonOtrProfile; offTheRecord: false }

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
            verify(!p.offTheRecord)

            compare(p.storageName, '')
            compare(p.cachePath, '')
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/UnknownProfile')
            compare(p.httpCacheType, WebEngineProfile.MemoryHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.NoPersistentCookies)

            compare(getPath(p.downloadPath), downloadLocation)
            compare(p.httpAcceptLanguage, '')
            verify(p.httpUserAgent !== '')
            compare(p.spellCheckEnabled, false)
            compare(p.spellCheckLanguages, [])

            compare(p.userScripts.collection, [])

            p.storageName = 'Test'
            compare(p.storageName, 'Test')
            compare(getPath(p.cachePath), cacheLocation + '/QtWebEngine/' + p.storageName)
            compare(getPath(p.persistentStoragePath), appDataLocation + '/QtWebEngine/' + p.storageName)

            compare(p.httpCacheType, WebEngineProfile.DiskHttpCache)
            compare(p.httpCacheMaximumSize, 0)
            compare(p.persistentCookiesPolicy, WebEngineProfile.AllowPersistentCookies)
        }
    }
}
