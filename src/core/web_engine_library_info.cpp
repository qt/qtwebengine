// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtwebenginecoreglobal_p.h"
#include "web_engine_library_info.h"

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "components/spellcheck/spellcheck_buildflags.h"
#include "content/public/common/content_paths.h"
#include "sandbox/policy/switches.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"

#include "type_conversion.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QLoggingCategory>
#include <QStandardPaths>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#ifndef QTWEBENGINEPROCESS_NAME
#error "No name defined for QtWebEngine's process"
#endif

using namespace Qt::StringLiterals;
using namespace QtWebEngineCore;

Q_WEBENGINE_LOGGING_CATEGORY(webEngineLibraryInfoLog, "qt.webengine.libraryinfo")

namespace {

QString fallbackDir() {
    static const QString directory =
            QDir::homePath() % "/."_L1 % QCoreApplication::applicationName();
    return directory;
}

#if defined(Q_OS_DARWIN) && defined(QT_MAC_FRAMEWORK_BUILD)
static inline CFBundleRef frameworkBundle()
{
    return CFBundleGetBundleWithIdentifier(CFSTR("org.qt-project.QtWebEngineCore"));
}

static QString getBundlePath(CFBundleRef frameworkBundle)
{
    QString path;
    // The following is a fix for QtWebEngineProcess crashes on OS X 10.7 and before.
    // We use it for the other OS X versions as well to make sure it works and because
    // the directory structure should be the same.
    if (qApp->applicationName() == QLatin1StringView(qWebEngineProcessName())) {
        path = QDir::cleanPath(qApp->applicationDirPath() % "/../../../.."_L1);
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
    if (qApp->applicationName() == QLatin1StringView(qWebEngineProcessName())) {
        path = getBundlePath(frameworkBundle) % "/Resources"_L1;
    } else if (frameworkBundle) {
        CFURLRef resourcesRelativeUrl = CFBundleCopyResourcesDirectoryURL(frameworkBundle);
        CFStringRef resourcesRelativePath = CFURLCopyFileSystemPath(resourcesRelativeUrl, kCFURLPOSIXPathStyle);
        path = getBundlePath(frameworkBundle) % QLatin1Char('/') % QString::fromCFString(resourcesRelativePath);
        CFRelease(resourcesRelativePath);
        CFRelease(resourcesRelativeUrl);
    }
    return path;
}
#endif

#if defined(Q_OS_DARWIN)
static QString getMainApplicationResourcesPath()
{
    QString resourcesPath;
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (!mainBundle)
        return resourcesPath;

    // Will point to Resources inside an app bundle, or in case if the application is not packaged
    // as a bundle, will point to the application directory, where the resources are assumed to be
    // found.
    CFURLRef resourcesRelativeUrl = CFBundleCopyResourcesDirectoryURL(mainBundle);
    if (!resourcesRelativeUrl)
        return resourcesPath;

    CFURLRef resourcesAbsoluteUrl = CFURLCopyAbsoluteURL(resourcesRelativeUrl);
    CFStringRef resourcesAbolutePath = CFURLCopyFileSystemPath(resourcesAbsoluteUrl,
                                                                kCFURLPOSIXPathStyle);
    resourcesPath = QString::fromCFString(resourcesAbolutePath);
    CFRelease(resourcesAbolutePath);
    CFRelease(resourcesAbsoluteUrl);
    CFRelease(resourcesRelativeUrl);

    return resourcesPath;
}

#endif

QString subProcessPath()
{
    static QString processPath;
    if (processPath.isEmpty()) {
#if defined(Q_OS_WIN)
        const QString processBinary = QLatin1StringView(qWebEngineProcessName()) % ".exe"_L1;
#else
        const auto processBinary = QLatin1StringView(qWebEngineProcessName());
#endif

        QStringList candidatePaths;
        bool includeOverrideMessage = false;
        if (QString fromEnv = qEnvironmentVariable("QTWEBENGINEPROCESS_PATH"); fromEnv.isEmpty()) {
            includeOverrideMessage = true;
#if defined(Q_OS_DARWIN) && defined(QT_MAC_FRAMEWORK_BUILD)
            candidatePaths << getBundlePath(frameworkBundle()) % "/Helpers/"_L1
                            % qWebEngineProcessName() % ".app/Contents/MacOS/"_L1
                            % qWebEngineProcessName();
#else
            candidatePaths << QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath)
                              % QLatin1Char('/') % processBinary;
            candidatePaths << QLibraryInfo::path(QLibraryInfo::BinariesPath)
                              % QLatin1Char('/') % processBinary;
#endif
            candidatePaths << QCoreApplication::applicationDirPath()
                              % QLatin1Char('/') % processBinary;
        } else {
            // Only search in QTWEBENGINEPROCESS_PATH if set
            candidatePaths.append(std::move(fromEnv));
        }

        for (const QString &candidate : std::as_const(candidatePaths)) {
            if (QFileInfo::exists(candidate)) {
                processPath = candidate;
                qCDebug(webEngineLibraryInfoLog, "Qt WebEngine process path: %ls",
                        qUtf16Printable(candidate));
                break;
            }
        }
        if (processPath.isEmpty()) {
            QString errorMessage;
            errorMessage += "The following paths were searched for Qt WebEngine Process:\n"_L1;
            for (const QString &candidate : std::as_const(candidatePaths))
                errorMessage += "  "_L1 + candidate + u'\n';
            errorMessage += "but could not find it.\n"_L1;
            if (includeOverrideMessage) {
                errorMessage += "You may override the default search path by using "
                                "QTWEBENGINEPROCESS_PATH environment variable.\n"_L1;
            }
            qFatal("%ls", qUtf16Printable(errorMessage));
        }

#if defined(Q_OS_WIN)
        base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();
        if (!parsedCommandLine->HasSwitch(sandbox::policy::switches::kNoSandbox)) {
            if (WebEngineLibraryInfo::isUNCPath(processPath) || WebEngineLibraryInfo::isRemoteDrivePath(processPath))
                qCritical("Can not launch QtWebEngineProcess from network path if sandbox is enabled: %s.", processPath.toUtf8().constData());
        }
#endif

    }


    return processPath;
}

QString localesPath()
{
    static QString potentialLocalesPath;
    if (potentialLocalesPath.isEmpty()) {
        QStringList candidatePaths;
        const QString translationPakFilename =
                QLatin1StringView(WebEngineLibraryInfo::getResolvedLocale()) % ".pak"_L1;
        bool includeOverrideMessage = false;
        if (QString fromEnv = qEnvironmentVariable("QTWEBENGINE_LOCALES_PATH"); fromEnv.isEmpty()) {
            includeOverrideMessage = true;
#if defined(Q_OS_DARWIN) && defined(QT_MAC_FRAMEWORK_BUILD)
            candidatePaths << getResourcesPath(frameworkBundle()) % QDir::separator()
                            % "qtwebengine_locales"_L1;
#endif
            candidatePaths << QLibraryInfo::path(QLibraryInfo::TranslationsPath) % QDir::separator()
                            % "qtwebengine_locales"_L1;
            candidatePaths << fallbackDir();
        } else {
            // Only search in QTWEBENGINE_LOCALES_PATH if set
            candidatePaths.append(std::move(fromEnv));
        }

        for (const QString &candidate : std::as_const(candidatePaths)) {
            if (QFileInfo::exists(candidate % QDir::separator() % translationPakFilename)) {
                potentialLocalesPath = candidate;
                qCDebug(webEngineLibraryInfoLog, "Qt WebEngine locales path: %ls",
                        qUtf16Printable(candidate));
                break;
            }
        }

        if (potentialLocalesPath.isEmpty()) {
            QString warningMessage;
            warningMessage += "The following paths were searched for Qt WebEngine locales:\n"_L1;
            for (const QString &candidate : std::as_const(candidatePaths))
                warningMessage += "  "_L1 % candidate + u'\n';
            warningMessage += "but could not find the translation file for the current locale: "_L1
                    + translationPakFilename + u'\n';
            if (includeOverrideMessage) {
                warningMessage += "You may override the default search paths by using "
                                  "QTWEBENGINE_LOCALES_PATH environment variable.\n"_L1;
            }
            warningMessage += "Translations WILL NOT be correct.\n"_L1;
            qWarning("%ls", qUtf16Printable(warningMessage));
        }
    }

    return potentialLocalesPath;
}

#if QT_CONFIG(webengine_spellchecker)
QString dictionariesPath(bool showWarnings)
{
    static QString potentialDictionariesPath;
    static QString warningMessage;
    static bool initialized = false;
    QStringList candidatePaths;
    if (!initialized) {
        initialized = true;

        bool includeOverrideMessage = false;
        if (QString fromEnv = qEnvironmentVariable("QTWEBENGINE_DICTIONARIES_PATH");
            fromEnv.isEmpty()) {
            includeOverrideMessage = true;
            // First try to find dictionaries near the application.
#ifdef Q_OS_DARWIN
            QString resourcesDictionariesPath = getMainApplicationResourcesPath()
                    % QDir::separator() % "qtwebengine_dictionaries"_L1;
            candidatePaths.append(std::move(resourcesDictionariesPath));
#endif
            QString applicationDictionariesPath = QCoreApplication::applicationDirPath()
                    % QDir::separator() % "qtwebengine_dictionaries"_L1;
            candidatePaths.append(std::move(applicationDictionariesPath));

            // Then try to find dictionaries near the installed library.
#if defined(Q_OS_DARWIN) && defined(QT_MAC_FRAMEWORK_BUILD)
            QString frameworkDictionariesPath =
                    getResourcesPath(frameworkBundle()) % "/qtwebengine_dictionaries"_L1;
            candidatePaths.append(std::move(frameworkDictionariesPath));
#endif

            QString libraryDictionariesPath = QLibraryInfo::path(QLibraryInfo::DataPath)
                    % QDir::separator() % "qtwebengine_dictionaries"_L1;
            candidatePaths.append(std::move(libraryDictionariesPath));
        } else {
            // Only search in QTWEBENGINE_DICTIONARIES_PATH if set
            candidatePaths.append(std::move(fromEnv));
        }

        for (const QString &candidate : std::as_const(candidatePaths)) {
            if (QFileInfo::exists(candidate)) {
                potentialDictionariesPath = candidate;
                qCDebug(webEngineLibraryInfoLog, "Qt WebEngine dictionaries path: %ls",
                        qUtf16Printable(candidate));
                break;
            }
        }
        if (potentialDictionariesPath.isEmpty()) {
            warningMessage +=
                    "The following paths were searched for Qt WebEngine dictionaries:\n"_L1;
            for (const QString &candidate : std::as_const(candidatePaths))
                warningMessage += "  "_L1 + candidate + u'\n';
            warningMessage += "but could not find it.\n"_L1;
            if (includeOverrideMessage) {
                warningMessage += "You may override the default search path by using "
                                  "QTWEBENGINE_DICTIONARIES_PATH environment variable.\n"_L1;
            }
            warningMessage += "Spellchecking can not be enabled.\n"_L1;
        }
    }

    if (showWarnings && !warningMessage.isEmpty()) {
        qWarning("%ls", qUtf16Printable(warningMessage));
    }

    return potentialDictionariesPath;
}
#endif // QT_CONFIG(webengine_spellchecker)

QString resourcesPath()
{
    static QString potentialResourcesPath;
    if (potentialResourcesPath.isEmpty()) {
        QStringList candidatePaths;
        const auto resourcesPakFilename = "qtwebengine_resources.pak"_L1;
        bool includeOverrideMessage = false;
        if (QString fromEnv = qEnvironmentVariable("QTWEBENGINE_RESOURCES_PATH");
            fromEnv.isEmpty()) {
            includeOverrideMessage = true;
#if defined(Q_OS_DARWIN) && defined(QT_MAC_FRAMEWORK_BUILD)
            candidatePaths << getResourcesPath(frameworkBundle());
#endif
            candidatePaths << QLibraryInfo::path(QLibraryInfo::DataPath) % QDir::separator()
                            % "resources"_L1;
            candidatePaths << QLibraryInfo::path(QLibraryInfo::DataPath);
            candidatePaths << QCoreApplication::applicationDirPath();
            candidatePaths << fallbackDir();
        } else {
            // Only search in QTWEBENGINE_RESOURCES_PATH if set
            candidatePaths.append(std::move(fromEnv));
        }

        for (const QString &candidate : std::as_const(candidatePaths)) {
            if (QFileInfo::exists(candidate % QDir::separator() % resourcesPakFilename)) {
                potentialResourcesPath = candidate;
                qCDebug(webEngineLibraryInfoLog, "Qt WebEngine resources path: %ls",
                        qUtf16Printable(candidate));
                break;
            }
        }

        if (potentialResourcesPath.isEmpty()) {
            QString errorMessage;
            errorMessage += "The following paths were searched for Qt WebEngine resources:\n"_L1;
            for (const QString &candidate : std::as_const(candidatePaths))
                errorMessage += "  "_L1 + candidate + u'\n';
            errorMessage += "but could not find any.\n"_L1;
            if (includeOverrideMessage) {
                errorMessage += "You may override the default search paths by using "
                                "QTWEBENGINE_RESOURCES_PATH environment variable.\n"_L1;
            }
            qFatal("%ls", qUtf16Printable(errorMessage));
        }
    }

    return potentialResourcesPath;
}
} // namespace

base::FilePath WebEngineLibraryInfo::getPath(int key, bool showWarnings)
{
    QString directory;
    switch (key) {
    case QT_RESOURCES_PAK:
        return toFilePath(resourcesPath() % "/qtwebengine_resources.pak"_L1);
    case QT_RESOURCES_100P_PAK:
        return toFilePath(resourcesPath() % "/qtwebengine_resources_100p.pak"_L1);
    case QT_RESOURCES_200P_PAK:
        return toFilePath(resourcesPath() % "/qtwebengine_resources_200p.pak"_L1);
    case QT_RESOURCES_DEVTOOLS_PAK:
        return toFilePath(resourcesPath() % "/qtwebengine_devtools_resources.pak"_L1);
#if defined(Q_OS_DARWIN) && defined(QT_MAC_FRAMEWORK_BUILD)
    case QT_FRAMEWORK_BUNDLE:
        return toFilePath(getBundlePath(frameworkBundle()));
#endif
    case base::FILE_EXE:
    case content::CHILD_PROCESS_EXE:
        return toFilePath(subProcessPath());
#if BUILDFLAG(IS_POSIX)
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
        return toFilePath(resourcesPath());
    case ui::DIR_LOCALES:
        return toFilePath(localesPath());
#if QT_CONFIG(webengine_spellchecker)
    case base::DIR_APP_DICTIONARIES:
        return toFilePath(dictionariesPath(showWarnings));
#endif
    case base::DIR_ASSETS:
        return toFilePath(resourcesPath());
    default:
        // Note: the path system expects this function to override the default
        // behavior. So no need to log an error if we don't support a given
        // path. The system will just use the default.
        return base::FilePath();
    }

    return toFilePath(directory.isEmpty() ? fallbackDir() : directory);
}

std::u16string WebEngineLibraryInfo::getApplicationName()
{
    return toString16(qApp->applicationName());
}

std::string WebEngineLibraryInfo::getResolvedLocale()
{
    base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();
    std::string locale;
    if (parsedCommandLine->HasSwitch(switches::kLang))
        locale = parsedCommandLine->GetSwitchValueASCII(switches::kLang);
    else
        locale = QLocale().bcp47Name().toStdString();

    std::string resolvedLocale;
    if (l10n_util::CheckAndResolveLocale(locale, &resolvedLocale))
        return resolvedLocale;

    return "en-US";
}

std::string WebEngineLibraryInfo::getApplicationLocale()
{
    base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();
    return parsedCommandLine->HasSwitch(switches::kLang)
        ? parsedCommandLine->GetSwitchValueASCII(switches::kLang)
        : QLocale().bcp47Name().toStdString();
}

#if defined(Q_OS_WIN)
bool WebEngineLibraryInfo::isRemoteDrivePath(const QString &path)
{
    WCHAR wDriveLetter[4] = { 0 };
    swprintf(wDriveLetter, 4, L"%S", path.mid(0, 3).toStdString().c_str());
    return GetDriveType(wDriveLetter) == DRIVE_REMOTE;
}

bool WebEngineLibraryInfo::isUNCPath(const QString &path)
{
    return (base::FilePath::IsSeparator(path.at(0).toLatin1())
            && base::FilePath::IsSeparator(path.at(1).toLatin1())
            && path.at(2) != QLatin1Char('.') && path.at(2) != QLatin1Char('?')
            && path.at(2).isLetter() && path.at(3) != QLatin1Char(':'));
}

#endif
