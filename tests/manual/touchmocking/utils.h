// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef UTILS_H
#define UTILS_H

#include <QFileInfo>
#include <QUrl>

class Utils : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE static QUrl fromUserInput(const QString &userInput);
};

inline QUrl Utils::fromUserInput(const QString &userInput)
{
    QFileInfo fileInfo(userInput);
    if (fileInfo.exists())
        return QUrl::fromLocalFile(fileInfo.absoluteFilePath());
    return QUrl::fromUserInput(userInput);
}

#endif // UTILS_H
