// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebengineprofile.h"
#include "qquickwebenginesettings_p.h"
#include "qquickwebenginesingleton_p.h"

#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebEngine
    //! \instantiates QQuickWebEngineSingleton
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.1
    \brief Provides access to the default settings and profiles shared by all web engine views.

    The WebEngine singleton type provides access to the default profile and the default settings
    shared by all web engine views. It can be used to change settings globally, as illustrated by
    the following code snippet:

    \code
    Component.onCompleted: {
        WebEngine.settings.pluginsEnabled = true;
    }
    \endcode
*/

/*!
    \qmlproperty WebEngineSettings WebEngine::settings
    \readonly
    \since QtWebEngine 1.1

    Default settings for all web engine views.

    \sa WebEngineSettings
*/

QQuickWebEngineSettings *QQuickWebEngineSingleton::settings() const
{
    return defaultProfile()->settings();
}

/*!
    \qmlproperty WebEngineProfile WebEngine::defaultProfile
    \readonly
    \since QtWebEngine 1.1

    Default profile for all web engine views.

    \sa WebEngineProfile
*/
QQuickWebEngineProfile *QQuickWebEngineSingleton::defaultProfile() const
{
    auto profile = QQuickWebEngineProfile::defaultProfile();

    // MEMO first ever call to default profile will create one without context
    // it needs something to get qml engine from (WebEngine singleton is created in qml land)
    profile->ensureQmlContext(this);

    return profile;
}

/*!
    \qmlmethod WebEngineScript WebEngine::script
    //! \instantiates QWebEngineScript
    \since QtWebEngine 6.2

    Constructs WebEngineScript, which can be set up and inserted into user scripts' collection
    for \l{WebEngineView::userScripts}{WebEngineView.userScripts} or
    \l{WebEngineProfile::userScripts}{WebEngineProfile.userScripts}
    using \l{WebEngineScriptCollection}.

    \sa WebEngineScript WebEngineScriptCollection
*/

QWebEngineScript QQuickWebEngineSingleton::script() const
{
    return QWebEngineScript();
}

QT_END_NAMESPACE

#include "moc_qquickwebenginesingleton_p.cpp"

