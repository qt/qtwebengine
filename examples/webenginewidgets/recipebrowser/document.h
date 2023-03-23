// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>
#include <QPlainTextEdit>

class Document : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_currentText NOTIFY textChanged FINAL)
public:
    explicit Document(QObject *parent = nullptr);

    void setTextEdit(QPlainTextEdit *textEdit);
    void setCurrentPage(const QString &page);

public slots:
    void setInitialText(const QString &text);
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);

private:
    QPlainTextEdit *m_textEdit;

    QString m_currentText;
    QString m_currentPage;
    QMap<QString, QString> m_textCollection;
};

#endif // DOCUMENT_H
