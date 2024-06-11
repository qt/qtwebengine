// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "document.h"

#include <QSettings>

Document::Document(QObject *parent) : QObject(parent)
{
    QSettings settings;
    settings.beginGroup("textCollection");
    QStringList pageTexts = settings.allKeys();
    for (const QString &name : std::as_const(pageTexts)) {
        QString pageText = settings.value(name).value<QString>();
        if (!pageText.isEmpty())
            m_textCollection.insert(name, pageText);
    }
    settings.endGroup();
}

void Document::setTextEdit(QPlainTextEdit *textEdit)
{
    m_textEdit = textEdit;
}

void Document::setCurrentPage(const QString &page)
{
    m_currentPage = page;
}

void Document::setInitialText(const QString &text)
{
    m_textEdit->setPlainText(m_textCollection.value(m_currentPage, text));
}

void Document::setText(const QString &text)
{
    if (text == m_currentText)
        return;
    m_currentText = text;
    emit textChanged(m_currentText);

    QSettings settings;
    settings.beginGroup("textCollection");
    settings.setValue(m_currentPage, text);
    m_textCollection.insert(m_currentPage, text);
    settings.endGroup();
}
