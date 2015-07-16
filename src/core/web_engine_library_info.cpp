/****************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#include "web_engine_library_info.h"

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "content/public/common/content_paths.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"
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

using namespace QtWebEngineCore;

namespace {

QString fallbackDir() {
    static QString directory = QDir::homePath() % QLatin1String("/.") % QCoreApplication::applicationName();
    return directory;
}

#if defined(OS_MACOSX)
static inline CFBundleRef frameworkBundle()
{
    return CFBundleGetBundleWithIdentifier(CFSTR("org.qt-project.Qt.QtWebEngineCore"));
}

static QString getPath(CFBundleRef frameworkBundle)
{
    QString path;
    // The following is a fix for QtWebEngineProcess crashes on OS X 10.7 and before.
    // We use it for the other OS X versions as well to make sure it works and because
    // the directory structure should be the same.
    if (qApp->applicationName() == QLatin1String(QTWEBENGINEPROCESS_NAME)) {
        path = QDir::cleanPath(qApp->applicationDirPath() % QLatin1String("/../../../.."));
    } else if (frameworkBundle) {
        CFURLRef bundleUrl = CFBundleCopyBundleURL(frameworkBundle);
        CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleUrl, kCFURLPOSIXPathStyle);
        path = QString::fromCFString(bundlePath);
        CFRelease(bundlePath);
        CFRelease(bundleUrl);
    }
    return path;
}

static QString getResourcesPath(CFBundleRef frameworkBundle)
{
    QString path;
    // The following is a fix for QtWebEngineProcess crashes on OS X 10.7 and before.
    // We use it for the other OS X versions as well to make sure it works and because
    // the directory structure should be the same.
    if (qApp->applicationName() == QLatin1String(QTWEBENGINEPROCESS_NAME)) {
        path = getPath(frameworkBundle) % QLatin1String("/Resources");
    } else if (frameworkBundle) {
        CFURLRef resourcesRelativeUrl = CFBundleCopyResourcesDirectoryURL(frameworkBundle);
        CFStringRef resourcesRelativePath = CFURLCopyFileSystemPath(resourcesRelativeUrl, kCFURLPOSIXPathStyle);
        path = getPath(frameworkBundle) % QLatin1Char('/') % QString::fromCFString(resourcesRelativePath);
        CFRelease(resourcesRelativePath);
        CFRelease(resourcesRelativeUrl);
    }
    return path;
}
#endif

QString subProcessPath()
{
    static QString processPath;
    if (processPath.isEmpty()) {
#if defined(OS_WIN)
        const QString processBinary = QLatin1String(QTWEBENGINEPROCESS_NAME) % QLatin1String(".exe");
#else
        const QString processBinary = QLatin1String(QTWEBENGINEPROCESS_NAME);
#endif

        QStringList candidatePaths;
        const QByteArray fromEnv = qgetenv("QTWEBENGINEPROCESS_PATH");
        if (!fromEnv.isEmpty()) {
            // Only search in QTWEBENGINEPROCESS_PATH if set
            candidatePaths << QString::fromLocal8Bit(fromEnv);
        } else {
#if defined(OS_MACOSX) && defined(QT_MAC_FRAMEWORK_BUILD)
            candidatePaths << getPath(frameworkBundle())
                              % QStringLiteral("/Helpers/" QTWEBENGINEPROCESS_NAME ".app/Contents/MacOS/" QTWEBENGINEPROCESS_NAME);
#else
            candidatePaths << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath)
                              % QLatin1Char('/') % processBinary;
            candidatePaths << QCoreApplication::applicationDirPath()
                              % QLatin1Char('/') % processBinary;
#endif
        }

        Q_FOREACH (const QString &candidate, candidatePaths) {
            if (QFileInfo(candidate).exists()) {
                processPath = candidate;
                break;
            }
        }
        if (processPath.isEmpty())
            qFatal("Could not find %s", processBinary.toUtf8().constData());

    }


    return processPath;
}

QString pluginsPath()
{
#if defined(OS_MACOSX) && defined(QT_MAC_FRAMEWORK_BUILD)
    static QString pluginsPath = getPath(frameworkBundle()) % QLatin1String("/Libraries");
#else
    static bool initialized = false;
    static QString pluginsPath;

    if (!initialized) {
        initialized = true;

        const QStringList directories = QCoreApplication::libraryPaths();
        Q_FOREACH (const QString &dir, directories) {
            const QString candidate = dir % "/" % QLatin1String("qtwebengine");
            if (QFileInfo(candidate).exists()) {
                pluginsPath = candidate;
                break;
            }
        }

        if (pluginsPath.isEmpty()) {
            qWarning("Qt WebEngine Plugins directory not found. Trying fallback directory... Plugins as for example video codecs MAY NOT work.");
            pluginsPath = fallbackDir();
        }
    }
#endif
    return pluginsPath;
}

QString localesPath()
{
#if defined(OS_MACOSX) && defined(QT_MAC_FRAMEWORK_BUILD)
    return getResourcesPath(frameworkBundle()) % QLatin1String("/qtwebengine_locales");
#else
    static bool initialized = false;
    static QString potentialLocalesPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath) % QDir::separator() % QLatin1String("qtwebengine_locales");

    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialLocalesPath)) {
            qWarning("Installed Qt WebEngine locales directory not found at location %s. Trying application directory...", qPrintable(potentialLocalesPath));
            potentialLocalesPath = QCoreApplication::applicationDirPath() % QDir::separator() % QLatin1String("qtwebengine_locales");
        }
        if (!QFileInfo::exists(potentialLocalesPath)) {
            qWarning("Qt WebEngine locales directory not found at location %s. Trying fallback directory... Translations MAY NOT not be correct.", qPrintable(potentialLocalesPath));
            potentialLocalesPath = fallbackDir();
        }
    }

    return potentialLocalesPath;
#endif
}

QString libraryDataPath()
{
#if defined(OS_MACOSX) && defined(QT_MAC_FRAMEWORK_BUILD)
    return getResourcesPath(frameworkBundle());
#else
    static bool initialized = false;
    static QString potentialDataPath = QLibraryInfo::location(QLibraryInfo::DataPath);
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialDataPath)) {
            qWarning("Qt WebEngine data directory not found at location %s. Trying application directory...", qPrintable(potentialDataPath));
            potentialDataPath = QCoreApplication::applicationDirPath();
        }
        if (!QFileInfo::exists(potentialDataPath)) {
            qWarning("Qt WebEngine data directory not found at location %s. Trying fallback directory... The application MAY NOT work.", qPrintable(potentialDataPath));
            potentialDataPath = fallbackDir();
        }
    }

    return potentialDataPath;
#endif
}
} // namespace

base::FilePath WebEngineLibraryInfo::getPath(int key)
{
    QString directory;
    switch (key) {
    case QT_RESOURCES_PAK:
        return toFilePath(libraryDataPath() % QLatin1String("/qtwebengine_resources.pak"));
    case QT_RESOURCES_100P_PAK:
        return toFilePath(libraryDataPath() % QLatin1String("/qtwebengine_resources_100p.pak"));
    case QT_RESOURCES_200P_PAK:
        return toFilePath(libraryDataPath() % QLatin1String("/qtwebengine_resources_200p.pak"));
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
        return toFilePath(libraryDataPath());
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

std::string WebEngineLibraryInfo::getApplicationLocale()
{
    CommandLine *parsedCommandLine = CommandLine::ForCurrentProcess();
    if (!parsedCommandLine->HasSwitch(switches::kLang))
        return QLocale().bcp47Name().toStdString();

    return parsedCommandLine->GetSwitchValueASCII(switches::kLang);
}
