/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
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

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QStringBuilder>

#ifndef QTWEBENGINEPROCESS_NAME
#error "No name defined for QtWebEngine's process"
#endif
QString WebEngineLibraryInfo::subProcessPath() {
    static bool initialized = false;
    static QString processPath (QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath)
                                % QDir::separator() % QStringLiteral(QTWEBENGINEPROCESS_NAME));
    if (initialized)
        return processPath;

    // Allow overriding at runtime for the time being.
    const QByteArray fromEnv = qgetenv("QTWEBENGINEPROCESS_PATH");
    if (!fromEnv.isEmpty())
        processPath = QString::fromLatin1(fromEnv);
    if (processPath.isEmpty() || !QFileInfo(processPath).exists())
        qFatal("QtWebEngineProcess not found at location %s. Try setting the QTWEBENGINEPROCESS_PATH environment variable.", qPrintable(processPath));
    initialized = true;
    return processPath;
}

QString WebEngineLibraryInfo::location(WebEngineLibraryInfo::LibraryLocation path)
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
    case TranslationsPath:
        if (!webEnginePath.isEmpty())
            return webEnginePath % QDir::separator() % QStringLiteral("translations");
        break;
    case DataPath:
        if (!webEnginePath.isEmpty())
            return webEnginePath;
        break;
    default:
        break;
    }
#endif
    return QLibraryInfo::location(static_cast<QLibraryInfo::LibraryLocation>(path));
}
