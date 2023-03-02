/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

