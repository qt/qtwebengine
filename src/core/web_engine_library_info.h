// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_ENGINE_LIBRARY_INFO_H
#define WEB_ENGINE_LIBRARY_INFO_H

#include "base/files/file_path.h"
#include <QString>

enum {
    QT_RESOURCES_PAK = 5000,
    QT_RESOURCES_100P_PAK = 5001,
    QT_RESOURCES_200P_PAK = 5002,
    QT_RESOURCES_DEVTOOLS_PAK = 5003,
    QT_FRAMEWORK_BUNDLE = 5004
};

class WebEngineLibraryInfo {
public:
    static base::FilePath getPath(int key);
    // Called by localized_error in our custom chrome layer
    static std::u16string getApplicationName();
    static std::string getResolvedLocale();
    static std::string getApplicationLocale();
#if defined(Q_OS_WIN)
    static bool isRemoteDrivePath(const QString &path);
    static bool isUNCPath(const QString &path);
#endif
};


#endif // WEB_ENGINE_LIBRARY_INFO_H
