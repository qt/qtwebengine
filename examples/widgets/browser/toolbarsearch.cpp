/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
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

#include "toolbarsearch.h"
#include "autosaver.h"

#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

#include <QtWidgets/QCompleter>
#include <QtWidgets/QMenu>
#include <QtCore/QStringListModel>

#include <QWebEngineSettings>

/*
    ToolbarSearch is a very basic search widget that also contains a small history.
    Searches are turned into urls that use Google to perform search
 */
ToolbarSearch::ToolbarSearch(QWidget *parent)
    : SearchLineEdit(parent)
    , m_autosaver(new AutoSaver(this))
    , m_maxSavedSearches(10)
    , m_stringListModel(new QStringListModel(this))
{
    QMenu *m = menu();
    connect(m, SIGNAL(aboutToShow()), this, SLOT(aboutToShowMenu()));
    connect(m, SIGNAL(triggered(QAction*)), this, SLOT(triggeredMenuAction(QAction*)));

    QCompleter *completer = new QCompleter(m_stringListModel, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    lineEdit()->setCompleter(completer);

    connect(lineEdit(), SIGNAL(returnPressed()), SLOT(searchNow()));
    setInactiveText(tr("Google"));
    load();
}

ToolbarSearch::~ToolbarSearch()
{
    m_autosaver->saveIfNeccessary();
}

void ToolbarSearch::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("toolbarsearch"));
    settings.setValue(QLatin1String("recentSearches"), m_stringListModel->stringList());
    settings.setValue(QLatin1String("maximumSaved"), m_maxSavedSearches);
    settings.endGroup();
}

void ToolbarSearch::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("toolbarsearch"));
    QStringList list = settings.value(QLatin1String("recentSearches")).toStringList();
    m_maxSavedSearches = settings.value(QLatin1String("maximumSaved"), m_maxSavedSearches).toInt();
    m_stringListModel->setStringList(list);
    settings.endGroup();
}

void ToolbarSearch::searchNow()
{
    QString searchText = lineEdit()->text();
    QStringList newList = m_stringListModel->stringList();
    if (newList.contains(searchText))
        newList.removeAt(newList.indexOf(searchText));
    newList.prepend(searchText);
    if (newList.size() >= m_maxSavedSearches)
        newList.removeLast();

    QWebEngineSettings *globalSettings = QWebEngineSettings::globalSettings();
    if (!globalSettings->testAttribute(QWebEngineSettings::PrivateBrowsingEnabled)) {
        m_stringListModel->setStringList(newList);
        m_autosaver->changeOccurred();
    }

    QUrl url(QLatin1String("http://www.google.com/search"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("q"), searchText);
    urlQuery.addQueryItem(QLatin1String("ie"), QLatin1String("UTF-8"));
    urlQuery.addQueryItem(QLatin1String("oe"), QLatin1String("UTF-8"));
    urlQuery.addQueryItem(QLatin1String("client"), QLatin1String("qtdemobrowser"));
    url.setQuery(urlQuery);
    emit search(url);
}

void ToolbarSearch::aboutToShowMenu()
{
    lineEdit()->selectAll();
    QMenu *m = menu();
    m->clear();
    QStringList list = m_stringListModel->stringList();
    if (list.isEmpty()) {
        m->addAction(tr("No Recent Searches"));
        return;
    }

    QAction *recent = m->addAction(tr("Recent Searches"));
    recent->setEnabled(false);
    for (int i = 0; i < list.count(); ++i) {
        QString text = list.at(i);
        m->addAction(text)->setData(text);
    }
    m->addSeparator();
    m->addAction(tr("Clear Recent Searches"), this, SLOT(clear()));
}

void ToolbarSearch::triggeredMenuAction(QAction *action)
{
    QVariant v = action->data();
    if (v.canConvert<QString>()) {
        QString text = v.toString();
        lineEdit()->setText(text);
        searchNow();
    }
}

void ToolbarSearch::clear()
{
    m_stringListModel->setStringList(QStringList());
    m_autosaver->changeOccurred();;
}
