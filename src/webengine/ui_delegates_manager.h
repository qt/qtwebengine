/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#ifndef UI_DELEGATES_MANAGER_H
#define UI_DELEGATES_MANAGER_H

#include "qglobal.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"

#include <QExplicitlySharedDataPointer>
#include <QPoint>
#include <QPointer>
#include <QQmlComponent>
#include <QUrl>

#define FOR_EACH_COMPONENT_TYPE(F, SEPARATOR) \
    F(Menu, menu) SEPARATOR \
    F(MenuItem, menuItem) SEPARATOR \
    F(MenuSeparator, menuSeparator) SEPARATOR \
    F(AlertDialog, alertDialog) SEPARATOR \
    F(ConfirmDialog, confirmDialog) SEPARATOR \
    F(PromptDialog, promptDialog) SEPARATOR \
    F(FilePicker, filePicker) SEPARATOR

#define COMMA_SEPARATOR ,
#define SEMICOLON_SEPARATOR ;
#define ENUM_DECLARATION(TYPE, COMPONENT) \
    TYPE
#define MEMBER_DECLARATION(TYPE, COMPONENT) \
    QScopedPointer<QQmlComponent> COMPONENT##Component

class JavaScriptDialogController;
class QQuickWebEngineView;

QT_FORWARD_DECLARE_CLASS(QObject)
QT_FORWARD_DECLARE_CLASS(QQmlContext)


class MenuItemData : public QObject {
Q_OBJECT
public:

    MenuItemData(const QString& text, const QString iconName = QString(), QObject *parent = 0);

    inline QString text() const { return m_text; }
    inline QString iconName() const { return m_iconName; }
    inline bool enabled() const { return m_enabled; }
    inline void setEnabled(bool on) { m_enabled = on; }

Q_SIGNALS:
    void triggered();

private:
    QString m_text;
    QString m_iconName;
    bool m_enabled;
};

class CopyMenuItem : public MenuItemData {
    Q_OBJECT
public:
    CopyMenuItem(const QString &itemText, const QString &textToCopy);

private:
    void onTriggered();

    QString m_textToCopy;

};

class NavigateMenuItem : public MenuItemData {
    Q_OBJECT
public:
    NavigateMenuItem(const QString &itemText, const QExplicitlySharedDataPointer<WebContentsAdapter> &adapter, const QUrl &targetUrl);

private:
    void onTriggered();

    QExplicitlySharedDataPointer<WebContentsAdapter> m_adapter;
    QUrl m_targetUrl;

};

class UIDelegatesManager {

public:
    enum ComponentType {
        Invalid = -1,
        FOR_EACH_COMPONENT_TYPE(ENUM_DECLARATION, COMMA_SEPARATOR)
        ComponentTypeCount
    };

    UIDelegatesManager(QQuickWebEngineView *);

    void addMenuItem(QObject *menu, MenuItemData *menuItem);
    void addMenuSeparator(QObject *menu);
    QObject *addMenu(QObject *parentMenu, const QString &title, const QPoint &pos = QPoint());
    QQmlContext *creationContextForComponent(QQmlComponent *);
    void showDialog(JavaScriptDialogController *);
    void showFilePicker(WebContentsAdapterClient::FileChooserMode, const QString &defaultFileName, const QString &title
                        , const QStringList &acceptedMimeTypes, const QExplicitlySharedDataPointer<WebContentsAdapter> &);

private:
    bool ensureComponentLoaded(ComponentType);
    QQmlComponent *loadDefaultUIDelegate(const QString &);

    QQuickWebEngineView *m_view;

    FOR_EACH_COMPONENT_TYPE(MEMBER_DECLARATION, SEMICOLON_SEPARATOR)

    Q_DISABLE_COPY(UIDelegatesManager);

};

#endif // UI_DELEGATES_MANAGER_H
