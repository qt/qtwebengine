/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginescript.h"

#include "user_script.h"
#include <QtCore/QDebug>

using QtWebEngineCore::UserScript;

/*!
    \class QWebEngineScript
    \inmodule QtWebEngineWidgets
    \since 5.5
    \brief The QWebEngineScript class encapsulates a JavaScript program.

    QWebEngineScript allows the programmatic injection of so called "user scripts" in the
    javascript engine at different points, determined by injectionPoint(), during the loading of web contents.

    Scripts can be executed either in the main javascript world, along with the rest of the JavaScript coming
    from the web contents, or in their own isolated world. While the DOM of the page can be accessed from any world,
    JavaScript variables a function defined in one world are not accessible from a different one.
    ScriptWorldId provides some predefined ids for this purpose.

*/
/*!
    \enum QWebEngineScript::InjectionPoint

    This enum describes the timing for when the script injection should happen.

    \value DocumentCreation The script will be executed as soon as the document is created. This is not suitable for
    any DOM operation.
    \value DocumentReady The script will run as soon as the DOM is ready. This is equivalent to the DOMContentLoaded
    event firing in JavaScript.
    \value Deferred The script will run when the page load finishes, or 500ms after the document is ready, whichever
    comes first.

*/
/*!
    \enum QWebEngineScript::ScriptWorldId

    This enum provides pre defined world ids for isolating user scripts into different worlds.

    \value MainWorld The world used by the page's web contents. It can be useful in order to expose custom functionality
    to web contents in certain scenarios.
    \value ApplicationWorld The default isolated world used for application level functionality implemented in JavaScript.
    \value UserWorld The first isolated world to be used by scripts set by users if the application is not making use
    of more worlds. As a rule of thumbs, if that functionality is exposed to the application users, each individual script
    should probably get its own isolated world.

*/

/*!
 * \brief QWebEngineScript::QWebEngineScript
 *
 * Constructs a null script.
 */

QWebEngineScript::QWebEngineScript()
    : d(new UserScript)
{
}
/*!
 * \brief QWebEngineScript::isNull
 * \return \c true is the script is null, \c false otherwise.
 */

QWebEngineScript::QWebEngineScript(const QWebEngineScript &other)
    : d(other.d)
{
}

QWebEngineScript::~QWebEngineScript()
{
}

QWebEngineScript &QWebEngineScript::operator=(const QWebEngineScript &other)
{
    d = other.d;
    return *this;
}

bool QWebEngineScript::isNull() const
{
    return d->isNull();
}

/*!
 * \brief QWebEngineScript::name
 * \return The name of the script. Can be useful to retrieve a given script from a QWebEngineScriptCollection.
 *
 * \sa QWebEngineScriptCollection::findScript(), QWebEngineScriptCollection::findScripts()
 */

QString QWebEngineScript::name() const
{
    return d->name();
}

/*!
 * \brief QWebEngineScript::setName
 * \param scriptName
 *
 * Sets the script name to \a scriptName.
 */
void QWebEngineScript::setName(const QString &scriptName)
{
    if (scriptName == name())
        return;
    d->setName(scriptName);
}

/*!
 * \brief QWebEngineScript::sourceCode
 * \return the source of the script.
 */
QString QWebEngineScript::sourceCode() const
{
    return d->sourceCode();
}

/*!
 * \brief QWebEngineScript::setSourceCode
 * \param scriptSource
 * Sets the script source to \a scriptSource.
 */
void QWebEngineScript::setSourceCode(const QString &scriptSource)
{
    if (scriptSource == sourceCode())
        return;
    d->setSourceCode(scriptSource);
}

ASSERT_ENUMS_MATCH(QWebEngineScript::Deferred, UserScript::AfterLoad)
ASSERT_ENUMS_MATCH(QWebEngineScript::DocumentReady, UserScript::DocumentLoadFinished)
ASSERT_ENUMS_MATCH(QWebEngineScript::DocumentCreation, UserScript::DocumentElementCreation)

/*!
 * \brief QWebEngineScript::injectionPoint
 * \return the point in the loading process at which the script will be executed.
 * The default value is QWebEngineScript::Deferred.
 *
 * \sa setInjectionPoint
 */
QWebEngineScript::InjectionPoint QWebEngineScript::injectionPoint() const
{
    return static_cast<QWebEngineScript::InjectionPoint>(d->injectionPoint());
}
/*!
 * \brief QWebEngineScript::setInjectionPoint
 * \param p
 * Sets the point at which to execute the script to be \p.
 *
 * \sa QWebEngineScript::InjectionPoint
 */
void QWebEngineScript::setInjectionPoint(QWebEngineScript::InjectionPoint p)
{
    if (p == injectionPoint())
        return;
    d->setInjectionPoint(static_cast<UserScript::InjectionPoint>(p));
}

/*!
 * \brief QWebEngineScript::worldId
 * \return the world id defining which world the script is executed in.
 */
quint32 QWebEngineScript::worldId() const
{
    return d->worldId();
}

/*!
 * \brief QWebEngineScript::setWorldId
 * \param id
 * Sets the world id of the isolated world to use when running this script.
 */
void QWebEngineScript::setWorldId(quint32 id)
{
    if (id == d->worldId())
        return;
    d->setWorldId(id);
}

/*!
 * \brief QWebEngineScript::runsOnSubFrames
 * \return \c true if the script is executed on every frame in the page, \c false if it is only ran for the main frame.
 */
bool QWebEngineScript::runsOnSubFrames() const
{
    return d->runsOnSubFrames();
}

/*!
 * \brief QWebEngineScript::setRunsOnSubFrames
 * \param on
 * Sets whether or not the script is executed on sub frames in addition to the main frame.
 */
void QWebEngineScript::setRunsOnSubFrames(bool on)
{
    if (runsOnSubFrames() == on)
        return;
    d->setRunsOnSubFrames(on);
}

/*!
 * \brief QWebEngineScript::operator ==
 * \param other
 * \return \c true if this QWebEngineScript is equal to \a other, otherwise returns \c false.
 */
bool QWebEngineScript::operator==(const QWebEngineScript &other) const
{
    return d == other.d || *d == *(other.d);
}

QWebEngineScript::QWebEngineScript(const UserScript &coreScript)
    : d(new UserScript(coreScript))
{
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QWebEngineScript &script)
{
    if (script.isNull())
        return d.maybeSpace() << "QWebEngineScript()";

    d.nospace() << "QWebEngineScript(" << script.name() << ", ";
    switch (script.injectionPoint()) {
    case QWebEngineScript::DocumentCreation:
        d << "QWebEngineScript::DocumentCreation" << ", ";
        break;
    case QWebEngineScript::DocumentReady:
        d << "QWebEngineScript::DocumentReady" << ", ";
        break;
    case QWebEngineScript::Deferred:
        d << "QWebEngineScript::Deferred" << ", ";
        break;
    }
    d << script.worldId() << ", "
      << script.runsOnSubFrames() << ", " << script.sourceCode() << ")";
    return d.space();
}
#endif
