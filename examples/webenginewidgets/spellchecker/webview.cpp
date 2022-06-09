// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "webview.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QWebEngineProfile>
#include <QWebEngineContextMenuRequest>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
{
    m_spellCheckLanguages["English"] = "en-US";
    m_spellCheckLanguages["German"] = "de-DE";
    QWebEngineProfile *profile = page()->profile();
    profile->setSpellCheckEnabled(true);
    profile->setSpellCheckLanguages({"en-US"});
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebEngineContextMenuRequest *data = lastContextMenuRequest();
    Q_ASSERT(data);

    if (!data->isContentEditable()) {
        QWebEngineView::contextMenuEvent(event);
        return;
    }

    QWebEngineProfile *profile = page()->profile();
    const QStringList &languages = profile->spellCheckLanguages();
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();

    QAction *spellcheckAction = new QAction(tr("Check Spelling"), nullptr);
    spellcheckAction->setCheckable(true);
    spellcheckAction->setChecked(profile->isSpellCheckEnabled());
    connect(spellcheckAction, &QAction::toggled, this, [profile](bool toogled) {
        profile->setSpellCheckEnabled(toogled);
    });
    menu->addAction(spellcheckAction);

    if (profile->isSpellCheckEnabled()) {
        QMenu *subMenu = menu->addMenu(tr("Select Language"));
        const QStringList keys = m_spellCheckLanguages.keys();
        for (const QString &str : keys) {
            QAction *action = subMenu->addAction(str);
            action->setCheckable(true);
            QString lang = m_spellCheckLanguages[str];
            action->setChecked(languages.contains(lang));
            connect(action, &QAction::triggered, this, [profile, lang](){
               profile->setSpellCheckLanguages(QStringList()<<lang);
            });
        }
    }
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(event->globalPos());
}


