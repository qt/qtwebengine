// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef UTILS_H
#define UTILS_H

#include <QtQml/qqml.h>

#include <QtCore/QFileInfo>
#include <QtCore/QUrl>

class Utils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
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
