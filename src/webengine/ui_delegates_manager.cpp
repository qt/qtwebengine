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

#include "ui_delegates_manager.h"

#include "api/qquickwebengineview_p.h"
#include "javascript_dialog_controller.h"

#include <QAbstractListModel>
#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QStringBuilder>
#include <private/qqmlmetatype_p.h>

// Uncomment for QML debugging
//#define UI_DELEGATES_DEBUG

#define NO_SEPARATOR
#define FILE_NAME_CASE_STATEMENT(TYPE, COMPONENT) \
    case UIDelegatesManager::TYPE:\
        return QStringLiteral(#TYPE".qml");

static QString fileNameForComponent(UIDelegatesManager::ComponentType type)
{
    switch (type) {
    FOR_EACH_COMPONENT_TYPE(FILE_NAME_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
    }
    return QString();
}

MenuItemHandler::MenuItemHandler(QObject *parent)
    : QObject(parent)
{
}


CopyMenuItem::CopyMenuItem(QObject *parent, const QString &textToCopy)
    : MenuItemHandler(parent)
    , m_textToCopy(textToCopy)
{
    connect(this, &MenuItemHandler::triggered, this, &CopyMenuItem::onTriggered);
}

void CopyMenuItem::onTriggered()
{
    qApp->clipboard()->setText(m_textToCopy);
}

NavigateMenuItem::NavigateMenuItem(QObject *parent, const QExplicitlySharedDataPointer<WebContentsAdapter> &adapter, const QUrl &targetUrl)
    : MenuItemHandler(parent)
    , m_adapter(adapter)
    , m_targetUrl(targetUrl)
{
    connect(this, &MenuItemHandler::triggered, this, &NavigateMenuItem::onTriggered);
}

void NavigateMenuItem::onTriggered()
{
    m_adapter->load(m_targetUrl);
}

UIDelegatesManager::UIDelegatesManager(QQuickWebEngineView *view)
    : m_view(view)
{
}

#define COMPONENT_MEMBER_CASE_STATEMENT(TYPE, COMPONENT) \
    case TYPE: \
        component = &COMPONENT##Component; \
        break;

bool UIDelegatesManager::ensureComponentLoaded(ComponentType type)
{
    QScopedPointer<QQmlComponent> *component;
    switch (type) {
    FOR_EACH_COMPONENT_TYPE(COMPONENT_MEMBER_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
        return false;
    }
    QString fileName(fileNameForComponent(type));
#ifndef UI_DELEGATES_DEBUG
    if (!(*component).isNull())
        return true;
#else // Unconditionally reload the components each time.
    fprintf(stderr, "%s: %s\n", Q_FUNC_INFO, qPrintable(fileName));
#endif
    QQmlEngine* engine = qmlEngine(m_view);
    if (!engine)
        return false;
    QString absolutePath;
    Q_FOREACH (const QString &path, engine->importPathList()) {
        QFileInfo fi(path % QStringLiteral("/QtWebEngine/UIDelegates/") % fileName);
        if (fi.exists())
            absolutePath = fi.absoluteFilePath();
    }
    // FIXME: handle async loading
    (*component).reset(new QQmlComponent(engine, QUrl(absolutePath), QQmlComponent::PreferSynchronous, m_view));

    if ((*component)->status() != QQmlComponent::Ready) {
#ifdef UI_DELEGATES_DEBUG
        Q_FOREACH (const QQmlError& err, (*component)->errors())
            fprintf(stderr, "  component error: %s\n", qPrintable(err.toString()));
#endif
        return false;
    }
    return true;
}

QQmlContext *UIDelegatesManager::creationContextForComponent(QQmlComponent *component)
{
    Q_ASSERT(component);

    QQmlContext* baseContext = component->creationContext() ? component->creationContext() : qmlContext(m_view);
    Q_ASSERT(baseContext);
    return baseContext;
}

#define CHECK_QML_SIGNAL_PROPERTY(prop, location) \
    if (!prop.isSignalProperty()) \
        qWarning("%s is missing %s signal property.\n", qPrintable(location.toString()), qPrintable(prop.name()));

void UIDelegatesManager::addMenuItem(MenuItemHandler *menuItemHandler, const QString &text, const QString &iconName, bool enabled)
{
    Q_ASSERT(menuItemHandler);
    if (!ensureComponentLoaded(MenuItem))
        return;
    QObject *it = menuItemComponent->beginCreate(creationContextForComponent(menuItemComponent.data()));

    QQmlProperty(it, QStringLiteral("text")).write(text);
    QQmlProperty(it, QStringLiteral("iconName")).write(iconName);
    QQmlProperty(it, QStringLiteral("enabled")).write(enabled);

    QQmlProperty signal(it, QStringLiteral("onTriggered"));
    CHECK_QML_SIGNAL_PROPERTY(signal, menuItemComponent->url());
    QObject::connect(it, signal.method(), menuItemHandler, QMetaMethod::fromSignal(&MenuItemHandler::triggered));
    menuItemComponent->completeCreate();

    QObject *menu = menuItemHandler->parent();
    it->setParent(menu);

    QQmlListReference entries(menu, QQmlMetaType::defaultProperty(menu).name(), qmlEngine(m_view));
    if (entries.isValid())
        entries.append(it);
}

void UIDelegatesManager::addMenuSeparator(QObject *menu)
{
    if (!ensureComponentLoaded(MenuSeparator))
        return;

    QQmlContext *itemContext = creationContextForComponent(menuSeparatorComponent.data());
    QObject *sep = menuSeparatorComponent->create(itemContext);
    sep->setParent(menu);

    QQmlListReference entries(menu, QQmlMetaType::defaultProperty(menu).name(), qmlEngine(m_view));
    if (entries.isValid())
        entries.append(sep);
}

QObject *UIDelegatesManager::addMenu(QObject *parentMenu, const QString &title, const QPoint& pos)
{

    if (!ensureComponentLoaded(Menu))
        return 0;
    QQmlContext *context(creationContextForComponent(menuComponent.data()));
    QObject *menu = menuComponent->beginCreate(context);
    // Useful when not using Qt Quick Controls' Menu
    if (QQuickItem* item = qobject_cast<QQuickItem*>(menu))
        item->setParentItem(m_view);

    if (!title.isEmpty())
        QQmlProperty(menu, QStringLiteral("title")).write(title);
    if (!pos.isNull())
        QQmlProperty(menu, QStringLiteral("pos")).write(pos);
    if (!parentMenu) {
        QQmlProperty doneSignal(menu, QStringLiteral("onDone"));
        static int deleteLaterIndex = menu->metaObject()->indexOfSlot("deleteLater()");
        if (doneSignal.isSignalProperty())
            QObject::connect(menu, doneSignal.method(), menu, menu->metaObject()->method(deleteLaterIndex));
    } else {
        menu->setParent(parentMenu);

        QQmlListReference entries(parentMenu, QQmlMetaType::defaultProperty(parentMenu).name(), qmlEngine(m_view));
        if (entries.isValid())
            entries.append(menu);
    }
    menuComponent->completeCreate();
    return menu;
}

#define ASSIGN_DIALOG_COMPONENT_DATA_CASE_STATEMENT(TYPE, COMPONENT) \
    case TYPE:\
        dialogComponent = COMPONENT##Component.data(); \
        break;


void UIDelegatesManager::showDialog(QSharedPointer<JavaScriptDialogController> dialogController)
{
    Q_ASSERT(!dialogController.isNull());
    ComponentType dialogComponentType = Invalid;
    QString title;
    switch (dialogController->type()) {
    case WebContentsAdapterClient::AlertDialog:
        dialogComponentType = AlertDialog;
        title = QObject::tr("Javascript Alert - %1");
        break;
    case WebContentsAdapterClient::ConfirmDialog:
        dialogComponentType = ConfirmDialog;
        title = QObject::tr("Javascript Confirm - %1");
        break;
    case WebContentsAdapterClient::PromptDialog:
        dialogComponentType = PromptDialog;
        title = QObject::tr("Javascript Prompt - %1");
        break;
    default:
        Q_UNREACHABLE();
    }

    if (!ensureComponentLoaded(dialogComponentType))
        return;

    QQmlComponent *dialogComponent = Q_NULLPTR;
    switch (dialogComponentType) {
    FOR_EACH_COMPONENT_TYPE(ASSIGN_DIALOG_COMPONENT_DATA_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
    }

    QQmlContext *context(creationContextForComponent(dialogComponent));
    QObject *dialog = dialogComponent->beginCreate(context);
    dialog->setParent(m_view);
    QQmlProperty textProp(dialog, QStringLiteral("text"));
    textProp.write(dialogController->message());

    QQmlProperty titleProp(dialog, QStringLiteral("title"));
    titleProp.write(title.arg(m_view->url().toString()));

    if (dialogComponentType == PromptDialog) {
        QQmlProperty promptProp(dialog, QStringLiteral("prompt"));
        promptProp.write(dialogController->defaultPrompt());
        QQmlProperty inputSignal(dialog, QStringLiteral("onInput"));
        CHECK_QML_SIGNAL_PROPERTY(inputSignal, dialogComponent->url());
        static int setTextIndex = dialogController->metaObject()->indexOfSlot("textProvided(QString)");
        QObject::connect(dialog, inputSignal.method(), dialogController.data(), dialogController->metaObject()->method(setTextIndex));
    }

    QQmlProperty acceptSignal(dialog, QStringLiteral("onAccepted"));
    QQmlProperty rejectSignal(dialog, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(acceptSignal, dialogComponent->url());
    CHECK_QML_SIGNAL_PROPERTY(rejectSignal, dialogComponent->url());

    static int acceptIndex = dialogController->metaObject()->indexOfSlot("accept()");
    QObject::connect(dialog, acceptSignal.method(), dialogController.data(), dialogController->metaObject()->method(acceptIndex));
    static int rejectIndex = dialogController->metaObject()->indexOfSlot("reject()");
    QObject::connect(dialog, rejectSignal.method(), dialogController.data(), dialogController->metaObject()->method(rejectIndex));
    dialogComponent->completeCreate();

    QObject::connect(dialogController.data(), &JavaScriptDialogController::dialogCloseRequested, dialog, &QObject::deleteLater);

    QMetaObject::invokeMethod(dialog, "open");
}
