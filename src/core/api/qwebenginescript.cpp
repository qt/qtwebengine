// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginescript.h"

#include "user_script.h"
#include <QtCore/QDebug>
#include <QtCore/QFile>

using QtWebEngineCore::UserScript;

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineScript
    \inmodule QtWebEngineCore
    \since 5.5
    \brief The QWebEngineScript class encapsulates a JavaScript program.


    QWebEngineScript enables the programmatic injection of so called \e {user scripts} in the
    JavaScript engine at different points, determined by injectionPoint(), during the loading of web
    contents.

    Scripts can be executed either in the main JavaScript \e world, along with the rest of the
    JavaScript coming from the web contents, or in their own isolated world. While the DOM of the
    page can be accessed from any world, JavaScript variables of a function defined in one world are
    not accessible from a different one. ScriptWorldId provides some predefined IDs for this
    purpose.

    The following \l Greasemonkey attributes are supported since Qt 5.8:
    \c @exclude, \c @include, \c @name, \c @match, and \c @run-at.

    Use QWebEnginePage::scripts() and QWebEngineProfile::scripts() to access
    the collection of scripts associated with a single page or a
    number of pages sharing the same profile.

    \sa {Script Injection}
*/
/*!
    \enum QWebEngineScript::InjectionPoint

    This enum describes the timing of the script injection:

    \value DocumentCreation The script will be executed as soon as the document is created. This is not suitable for
    any DOM operation.
    \value DocumentReady The script will run as soon as the DOM is ready. This is equivalent to the
    \c DOMContentLoaded event firing in JavaScript.
    \value Deferred The script will run when the page load finishes, or 500ms after the document is ready, whichever
    comes first.

*/
/*!
    \enum QWebEngineScript::ScriptWorldId

    This enum provides pre-defined world IDs for isolating user scripts into different worlds:

    \value MainWorld The world used by the page's web contents. It can be useful in order to expose custom functionality
    to web contents in certain scenarios.
    \value ApplicationWorld The default isolated world used for application level functionality implemented in JavaScript.
    \value UserWorld The first isolated world to be used by scripts set by users if the application is not making use
    of more worlds. As a rule of thumb, if that functionality is exposed to the application users, each individual script
    should probably get its own isolated world.

*/

/*!
    \fn QWebEngineScript::operator!=(const QWebEngineScript &other) const

    Returns \c true if the script is not equal to \a other, otherwise returns \c false.
*/

/*!
    \fn QWebEngineScript::swap(QWebEngineScript &other)

     Swaps the contents of the script with the contents of \a other.
*/

/*!
 * Constructs a null script.
 */

QWebEngineScript::QWebEngineScript()
    : d(new UserScript)
{
}

/*!
 * Constructs a user script using the contents of \a other.
 */
QWebEngineScript::QWebEngineScript(const QWebEngineScript &other)
    : d(other.d)
{
}

/*!
    Destroys a script.
*/
QWebEngineScript::~QWebEngineScript()
{
}

/*!
    Assigns \a other to the script.
*/
QWebEngineScript &QWebEngineScript::operator=(const QWebEngineScript &other)
{
    d = other.d;
    return *this;
}

/*!
 * Returns the name of the script. Can be useful to retrieve a particular script from a
 * QWebEngineScriptCollection.
 *
 * \sa QWebEngineScriptCollection::find()
 */

QString QWebEngineScript::name() const
{
    return d->name();
}

/*!
 * Sets the script name to \a scriptName.
 */
void QWebEngineScript::setName(const QString &scriptName)
{
    if (scriptName == name())
        return;
    d->setName(scriptName);
}


QUrl QWebEngineScript::sourceUrl() const
{
    return d->sourceUrl();
}

void QWebEngineScript::setSourceUrl(const QUrl &url)
{
    if (url == sourceUrl())
        return;

    d->setSourceUrl(url);

    QFile file;
    if (url.isLocalFile()) {
        file.setFileName(url.toLocalFile());
    } else if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
        if (url.authority().isEmpty())
            file.setFileName(QLatin1Char(':') + url.path());
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open user script " << url;
        return;
    }

    QString source = QString::fromUtf8(file.readAll());
    setSourceCode(source);
}

/*!
    Returns the source of the script.
 */
QString QWebEngineScript::sourceCode() const
{
    return d->sourceCode();
}

/*!
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
 * Returns the point in the loading process at which the script will be executed.
 * The default value is QWebEngineScript::Deferred.
 *
 * \sa setInjectionPoint()
 */
QWebEngineScript::InjectionPoint QWebEngineScript::injectionPoint() const
{
    return static_cast<QWebEngineScript::InjectionPoint>(d->injectionPoint());
}
/*!
 * Sets the point at which to execute the script to be \a p.
 *
 * \sa InjectionPoint
 */
void QWebEngineScript::setInjectionPoint(QWebEngineScript::InjectionPoint p)
{
    if (p == injectionPoint())
        return;
    d->setInjectionPoint(static_cast<UserScript::InjectionPoint>(p));
}

/*!
    Returns the world ID defining which world the script is executed in.
 */
quint32 QWebEngineScript::worldId() const
{
    return d->worldId();
}

/*!
    Sets the world ID of the isolated world to \a id when running this script.

    Must be between \c 0 and \c 256.
 */
void QWebEngineScript::setWorldId(quint32 id)
{
    if (id == d->worldId())
        return;
    d->setWorldId(id);
}

/*!
    Returns \c true if the script is executed on every frame in the page, or \c false if it is only
    ran for the main frame.
 */
bool QWebEngineScript::runsOnSubFrames() const
{
    return d->runsOnSubFrames();
}

/*!
 * Executes the script on sub frames in addition to the main frame if \a on returns \c true.
 */
void QWebEngineScript::setRunsOnSubFrames(bool on)
{
    if (runsOnSubFrames() == on)
        return;
    d->setRunsOnSubFrames(on);
}

/*!
    Returns \c true if the script is equal to \a other, otherwise returns \c false.
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

QT_END_NAMESPACE

#include "moc_qwebenginescript.cpp"
