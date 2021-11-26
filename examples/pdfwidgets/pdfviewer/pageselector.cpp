/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "pageselector.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPdfPageNavigation>
#include <QToolButton>

PageSelector::PageSelector(QWidget *parent)
    : QWidget(parent)
    , m_pageNavigation(nullptr)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    m_previousPageButton = new QToolButton(this);
    m_previousPageButton->setText("<");
    m_previousPageButton->setEnabled(false);

    m_pageNumberEdit = new QLineEdit(this);
    m_pageNumberEdit->setAlignment(Qt::AlignRight);

    m_pageCountLabel = new QLabel(this);
    m_pageCountLabel->setText("0");

    m_nextPageButton = new QToolButton(this);
    m_nextPageButton->setText(">");
    m_nextPageButton->setEnabled(false);

    layout->addWidget(m_previousPageButton);
    layout->addWidget(m_pageNumberEdit);
    layout->addWidget(m_pageCountLabel);
    layout->addWidget(m_nextPageButton);
}

void PageSelector::setPageNavigation(QPdfPageNavigation *pageNavigation)
{
    m_pageNavigation = pageNavigation;

    connect(m_previousPageButton, &QToolButton::clicked, m_pageNavigation, &QPdfPageNavigation::goToPreviousPage);
    connect(m_pageNavigation, &QPdfPageNavigation::canGoToPreviousPageChanged, m_previousPageButton, &QToolButton::setEnabled);

    connect(m_pageNavigation, &QPdfPageNavigation::currentPageChanged, this, &PageSelector::onCurrentPageChanged);
    connect(m_pageNavigation, &QPdfPageNavigation::pageCountChanged, this, [this](int pageCount){ m_pageCountLabel->setText(QString::fromLatin1("/ %1").arg(pageCount)); });

    connect(m_pageNumberEdit, &QLineEdit::editingFinished, this, &PageSelector::pageNumberEdited);

    connect(m_nextPageButton, &QToolButton::clicked, m_pageNavigation, &QPdfPageNavigation::goToNextPage);
    connect(m_pageNavigation, &QPdfPageNavigation::canGoToNextPageChanged, m_nextPageButton, &QToolButton::setEnabled);

    onCurrentPageChanged(m_pageNavigation->currentPage());
}

void PageSelector::onCurrentPageChanged(int page)
{
    if (m_pageNavigation->pageCount() == 0)
        m_pageNumberEdit->setText(QString::number(0));
    else
        m_pageNumberEdit->setText(QString::number(page + 1));
}

void PageSelector::pageNumberEdited()
{
    if (!m_pageNavigation)
        return;

    const QString text = m_pageNumberEdit->text();

    bool ok = false;
    const int pageNumber = text.toInt(&ok);

    if (!ok)
        onCurrentPageChanged(m_pageNavigation->currentPage());
    else
        m_pageNavigation->setCurrentPage(qBound(0, pageNumber - 1, m_pageNavigation->pageCount() - 1));
}
