// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef UTILS_H
#define UTILS_H

#include <QtCore/QFileInfo>
#include <QtCore/QUrl>

class Utils : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE static QUrl fromUserInput(const QString& userInput);
    Q_INVOKABLE static QString domainFromString(const QString& urlString);
};

inline QUrl Utils::fromUserInput(const QString& userInput)
{
    QFileInfo fileInfo(userInput);
    if (fileInfo.exists())
        return QUrl::fromLocalFile(fileInfo.absoluteFilePath());
    return QUrl::fromUserInput(userInput);
}

inline QString Utils::domainFromString(const QString& urlString)
{
    return QUrl::fromUserInput(urlString).host();
}

#endif // UTILS_H
