// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>

class Document : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged FINAL)
public:
    explicit Document(QObject *parent = nullptr) : QObject(parent) {}

    void setText(const QString &text);

signals:
    void textChanged(const QString &text);

private:
    QString m_text;
};

#endif // DOCUMENT_H
