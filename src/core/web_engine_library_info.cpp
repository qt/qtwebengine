/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#include "web_engine_library_info.h"

#include "base/base_paths.h"
#include "base/file_util.h"
#include "content/public/common/content_paths.h"
#include "ui/base/ui_base_paths.h"
#include "type_conversion.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>

#ifndef QTWEBENGINEPROCESS_NAME
#error "No name defined for QtWebEngine's process"
#endif


namespace {

QString location(QLibraryInfo::LibraryLocation path)
{
#if defined(Q_OS_BLACKBERRY)
    // On BlackBerry, the qtwebengine may live in /usr/lib/qtwebengine.
    // If so, the QTWEBENGINEPROCESS_PATH env var is set to /usr/lib/qtwebengine/bin/QTWEBENGINEPROCESS_NAME.
    static QString webEnginePath;
    static bool initialized = false;
    if (!initialized) {
        const QByteArray fromEnv = qgetenv("QTWEBENGINEPROCESS_PATH");
        if (!fromEnv.isEmpty()) {
            QDir dir = QFileInfo(QString::fromLatin1(fromEnv)).dir();
            if (dir.cdUp())
                webEnginePath = dir.absolutePath();
        }
        initialized = true;
    }
    switch (path) {
    case QLibraryInfo::TranslationsPath:
        if (!webEnginePath.isEmpty())
            return webEnginePath % QDir::separator() % QStringLiteral("translations");
        break;
    case QLibraryInfo::DataPath:
        if (!webEnginePath.isEmpty())
            return webEnginePath;
        break;
    default:
        break;
    }
#endif

    return QLibraryInfo::location(path);
}

QString subProcessPath()
{
    static bool initialized = false;
#if defined(OS_WIN)
    static QString processPath (location(QLibraryInfo::LibraryExecutablesPath)
                                % QDir::separator() % QStringLiteral(QTWEBENGINEPROCESS_NAME) % QStringLiteral(".exe"));
#else
    static QString processPath (location(QLibraryInfo::LibraryExecutablesPath)
                                % QDir::separator() % QStringLiteral(QTWEBENGINEPROCESS_NAME));
#endif
    if (!initialized) {
        // Allow overriding at runtime for the time being.
        const QByteArray fromEnv = qgetenv("QTWEBENGINEPROCESS_PATH");
        if (!fromEnv.isEmpty())
            processPath = QString::fromLatin1(fromEnv);
        if (processPath.isEmpty() || !QFileInfo(processPath).exists())
            qFatal("QtWebEngineProcess not found at location %s. Try setting the QTWEBENGINEPROCESS_PATH environment variable.", qPrintable(processPath));
        initialized = true;
    }

    return processPath;
}

QString pluginsPath()
{
    return location(QLibraryInfo::PluginsPath) % QDir::separator() % QStringLiteral("qtwebengine");
}

QString localesPath()
{
    return location(QLibraryInfo::TranslationsPath) % QStringLiteral("/qtwebengine_locales");
}

QString fallbackDir() {
    static QString directory = QDir::homePath() % QDir::separator() % QChar::fromLatin1('.') % QCoreApplication::applicationName();
    return directory;
}

} // namespace

#if defined(OS_ANDROID)
namespace base {
// Replace the Android base path provider that depends on jni.
// With this we avoid patching chromium which we would need since
// PathService registers PathProviderAndroid by default on Android.
bool PathProviderAndroid(int key, FilePath* result)
{
    *result = WebEngineLibraryInfo::getPath(key);
    return !(result->empty());
}

}
#endif // defined(OS_ANDROID)

base::FilePath WebEngineLibraryInfo::getPath(int key)
{
    QString directory;
    switch (key) {
    case QT_RESOURCES_PAK:
        return toFilePath(location(QLibraryInfo::DataPath) % QStringLiteral("/qtwebengine_resources.pak"));
    case base::FILE_EXE:
    case content::CHILD_PROCESS_EXE:
        return toFilePath(subProcessPath());
#if defined(OS_POSIX)
    case base::DIR_CACHE:
        directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        break;
    case base::DIR_HOME:
        directory = QDir::homePath();
        break;
#endif
    case base::DIR_USER_DESKTOP:
        directory = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        break;
    case base::DIR_QT_LIBRARY_DATA:
        return toFilePath(location(QLibraryInfo::DataPath));
#if defined(OS_ANDROID)
    case base::DIR_SOURCE_ROOT:
    case base::DIR_ANDROID_EXTERNAL_STORAGE:
    case base::DIR_ANDROID_APP_DATA:
        directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        break;
#endif
    case content::DIR_MEDIA_LIBS:
        return toFilePath(pluginsPath());
    case ui::DIR_LOCALES:
        return toFilePath(localesPath());
    default:
        // Note: the path system expects this function to override the default
        // behavior. So no need to log an error if we don't support a given
        // path. The system will just use the default.
        return base::FilePath();
    }

    return toFilePath(directory.isEmpty() ? fallbackDir() : directory);
}

base::string16 WebEngineLibraryInfo::getApplicationName()
{
    return toString16(qApp->applicationName());
}
