/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ui_delegates_manager.h"

#include "api/qquickwebengineview_p.h"
#include <authentication_dialog_controller.h>
#include <color_chooser_controller.h>
#include <file_picker_controller.h>
#include <javascript_dialog_controller.h>
#include <web_contents_adapter_client.h>

#include <QFileInfo>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>

// Uncomment for QML debugging
//#define UI_DELEGATES_DEBUG

namespace QtWebEngineCore {

#define NO_SEPARATOR
#if defined(Q_OS_WIN)
#define FILE_NAME_CASE_STATEMENT(TYPE, COMPONENT) \
    case UIDelegatesManager::TYPE:\
        return QString::fromLatin1(#TYPE ##".qml");
#else
#define FILE_NAME_CASE_STATEMENT(TYPE, COMPONENT) \
    case UIDelegatesManager::TYPE:\
        return QStringLiteral(#TYPE".qml");
#endif

static QString fileNameForComponent(UIDelegatesManager::ComponentType type)
{
    switch (type) {
    FOR_EACH_COMPONENT_TYPE(FILE_NAME_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
    }
    return QString();
}

static QString getUIDelegatesImportDir(QQmlEngine *engine) {
    static QString importDir;
    static bool initialized = false;
    if (initialized)
        return importDir;
    Q_FOREACH (const QString &path, engine->importPathList()) {
        QFileInfo fi(path % QLatin1String("/QtWebEngine/UIDelegates/"));
        if (fi.exists()) {
            importDir = fi.absolutePath();
            break;
        }
    }
    initialized = true;
    return importDir;
}

const char *defaultPropertyName(QObject *obj)
{
    const QMetaObject *metaObject = obj->metaObject();

    int idx = metaObject->indexOfClassInfo("DefaultProperty");
    if (-1 == idx)
        return 0;

    QMetaClassInfo info = metaObject->classInfo(idx);
    return info.value();
}

MenuItemHandler::MenuItemHandler(QObject *parent)
    : QObject(parent)
{
}

#define COMPONENT_MEMBER_INIT(TYPE, COMPONENT) \
    , COMPONENT##Component(0)

UIDelegatesManager::UIDelegatesManager(QQuickWebEngineView *view)
    : m_view(view)
    , m_messageBubbleItem(0)
    FOR_EACH_COMPONENT_TYPE(COMPONENT_MEMBER_INIT, NO_SEPARATOR)
{
}

#define COMPONENT_MEMBER_CASE_STATEMENT(TYPE, COMPONENT) \
    case TYPE: \
        component = &COMPONENT##Component; \
        break;

bool UIDelegatesManager::ensureComponentLoaded(ComponentType type)
{
    QQmlEngine* engine = qmlEngine(m_view);
    if (getUIDelegatesImportDir(engine).isNull())
        return false;
    QQmlComponent **component;
    switch (type) {
    FOR_EACH_COMPONENT_TYPE(COMPONENT_MEMBER_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
        return false;
    }
    QString fileName(fileNameForComponent(type));
#ifndef UI_DELEGATES_DEBUG
    if (*component)
        return true;
#else // Unconditionally reload the components each time.
    fprintf(stderr, "%s: %s\n", Q_FUNC_INFO, qPrintable(fileName));
#endif
    if (!engine)
        return false;
    QFileInfo fi(getUIDelegatesImportDir(engine) % QLatin1Char('/') % fileName);
    if (!fi.exists())
        return false;
    // FIXME: handle async loading
    *component = (new QQmlComponent(engine, QUrl::fromLocalFile(fi.absoluteFilePath()), QQmlComponent::PreferSynchronous, m_view));

    if ((*component)->status() != QQmlComponent::Ready) {
        Q_FOREACH (const QQmlError& err, (*component)->errors())
            qWarning("QtWebEngine: component error: %s\n", qPrintable(err.toString()));
        delete *component;
        *component = 0;
        return false;
    }
    return true;
}

#define CHECK_QML_SIGNAL_PROPERTY(prop, location) \
    if (!prop.isSignalProperty()) \
        qWarning("%s is missing %s signal property.\n", qPrintable(location.toString()), qPrintable(prop.name()));

void UIDelegatesManager::addMenuItem(MenuItemHandler *menuItemHandler, const QString &text, const QString &iconName, bool enabled,
                                     bool checkable, bool checked)
{
    Q_ASSERT(menuItemHandler);
    if (!ensureComponentLoaded(MenuItem))
        return;
    QObject *it = menuItemComponent->beginCreate(qmlContext(m_view));

    QQmlProperty(it, QStringLiteral("text")).write(text);
    QQmlProperty(it, QStringLiteral("iconName")).write(iconName);
    QQmlProperty(it, QStringLiteral("enabled")).write(enabled);
    QQmlProperty(it, QStringLiteral("checkable")).write(checkable);
    QQmlProperty(it, QStringLiteral("checked")).write(checked);

    QQmlProperty signal(it, QStringLiteral("onTriggered"));
    CHECK_QML_SIGNAL_PROPERTY(signal, menuItemComponent->url());
    QObject::connect(it, signal.method(), menuItemHandler, QMetaMethod::fromSignal(&MenuItemHandler::triggered));
    menuItemComponent->completeCreate();

    QObject *menu = menuItemHandler->parent();
    it->setParent(menu);

    QQmlListReference entries(menu, defaultPropertyName(menu), qmlEngine(m_view));
    if (entries.isValid())
        entries.append(it);
}

void UIDelegatesManager::addMenuSeparator(QObject *menu)
{
    if (!ensureComponentLoaded(MenuSeparator))
        return;

    QQmlContext *itemContext = qmlContext(m_view);
    QObject *sep = menuSeparatorComponent->create(itemContext);
    sep->setParent(menu);

    QQmlListReference entries(menu, defaultPropertyName(menu), qmlEngine(m_view));
    if (entries.isValid())
        entries.append(sep);
}

QObject *UIDelegatesManager::addMenu(QObject *parentMenu, const QString &title, const QPoint& pos)
{

    if (!ensureComponentLoaded(Menu))
        return 0;
    QQmlContext *context = qmlContext(m_view);
    QObject *menu = menuComponent->beginCreate(context);
    // set visual parent for non-Window-based menus
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

        QQmlListReference entries(parentMenu, defaultPropertyName(parentMenu), qmlEngine(m_view));
        if (entries.isValid())
            entries.append(menu);
    }
    menuComponent->completeCreate();
    return menu;
}

#define ASSIGN_DIALOG_COMPONENT_DATA_CASE_STATEMENT(TYPE, COMPONENT) \
    case TYPE:\
        dialogComponent = COMPONENT##Component; \
        break;


void UIDelegatesManager::showDialog(QSharedPointer<JavaScriptDialogController> dialogController)
{
    Q_ASSERT(!dialogController.isNull());
    ComponentType dialogComponentType = Invalid;
    QString title;
    switch (dialogController->type()) {
    case WebContentsAdapterClient::AlertDialog:
        dialogComponentType = AlertDialog;
        title = tr("Javascript Alert - %1").arg(m_view->url().toString());
        break;
    case WebContentsAdapterClient::ConfirmDialog:
        dialogComponentType = ConfirmDialog;
        title = tr("Javascript Confirm - %1").arg(m_view->url().toString());
        break;
    case WebContentsAdapterClient::PromptDialog:
        dialogComponentType = PromptDialog;
        title = tr("Javascript Prompt - %1").arg(m_view->url().toString());
        break;
    case WebContentsAdapterClient::UnloadDialog:
        dialogComponentType = ConfirmDialog;
        title = tr("Are you sure you want to leave this page?");
        break;
    case WebContentsAdapterClient::InternalAuthorizationDialog:
        dialogComponentType = ConfirmDialog;
        title = dialogController->title();
        break;
    default:
        Q_UNREACHABLE();
    }

    if (!ensureComponentLoaded(dialogComponentType)) {
        // Let the controller know it couldn't be loaded
        qWarning("Failed to load dialog, rejecting.");
        dialogController->reject();
        return;
    }

    QQmlComponent *dialogComponent = Q_NULLPTR;
    switch (dialogComponentType) {
    FOR_EACH_COMPONENT_TYPE(ASSIGN_DIALOG_COMPONENT_DATA_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
    }

    QQmlContext *context = qmlContext(m_view);
    QObject *dialog = dialogComponent->beginCreate(context);
    // set visual parent for non-Window-based dialogs
    if (QQuickItem* item = qobject_cast<QQuickItem*>(dialog))
        item->setParentItem(m_view);
    dialog->setParent(m_view);
    QQmlProperty textProp(dialog, QStringLiteral("text"));
    textProp.write(dialogController->message());

    QQmlProperty titleProp(dialog, QStringLiteral("title"));
    titleProp.write(title);

    QQmlProperty acceptSignal(dialog, QStringLiteral("onAccepted"));
    QQmlProperty rejectSignal(dialog, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(acceptSignal, dialogComponent->url());
    CHECK_QML_SIGNAL_PROPERTY(rejectSignal, dialogComponent->url());

    static int acceptIndex = dialogController->metaObject()->indexOfSlot("accept()");
    QObject::connect(dialog, acceptSignal.method(), dialogController.data(), dialogController->metaObject()->method(acceptIndex));
    static int rejectIndex = dialogController->metaObject()->indexOfSlot("reject()");
    QObject::connect(dialog, rejectSignal.method(), dialogController.data(), dialogController->metaObject()->method(rejectIndex));

    if (dialogComponentType == PromptDialog) {
        QQmlProperty promptProp(dialog, QStringLiteral("prompt"));
        promptProp.write(dialogController->defaultPrompt());
        QQmlProperty inputSignal(dialog, QStringLiteral("onInput"));
        CHECK_QML_SIGNAL_PROPERTY(inputSignal, dialogComponent->url());
        static int setTextIndex = dialogController->metaObject()->indexOfSlot("textProvided(QString)");
        QObject::connect(dialog, inputSignal.method(), dialogController.data(), dialogController->metaObject()->method(setTextIndex));
        QQmlProperty closingSignal(dialog, QStringLiteral("onClosing"));
        QObject::connect(dialog, closingSignal.method(), dialogController.data(), dialogController->metaObject()->method(rejectIndex));
    }

    dialogComponent->completeCreate();

    QObject::connect(dialogController.data(), &JavaScriptDialogController::dialogCloseRequested, dialog, &QObject::deleteLater);

    QMetaObject::invokeMethod(dialog, "open");
}

void UIDelegatesManager::showColorDialog(QSharedPointer<ColorChooserController> controller)
{
    if (!ensureComponentLoaded(ColorDialog)) {
        // Let the controller know it couldn't be loaded
        qWarning("Failed to load dialog, rejecting.");
        controller->reject();
        return;
    }

    QQmlContext *context = qmlContext(m_view);
    QObject *colorDialog = colorDialogComponent->beginCreate(context);
    if (QQuickItem* item = qobject_cast<QQuickItem*>(colorDialog))
        item->setParentItem(m_view);
    colorDialog->setParent(m_view);

    if (controller->initialColor().isValid())
        colorDialog->setProperty("color", controller->initialColor());

    QQmlProperty selectedColorSignal(colorDialog, QStringLiteral("onSelectedColor"));
    CHECK_QML_SIGNAL_PROPERTY(selectedColorSignal, colorDialogComponent->url());
    QQmlProperty rejectedSignal(colorDialog, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(rejectedSignal, colorDialogComponent->url());

    static int acceptIndex = controller->metaObject()->indexOfSlot("accept(QVariant)");
    QObject::connect(colorDialog, selectedColorSignal.method(), controller.data(), controller->metaObject()->method(acceptIndex));
    static int rejectIndex = controller->metaObject()->indexOfSlot("reject()");
    QObject::connect(colorDialog, rejectedSignal.method(), controller.data(), controller->metaObject()->method(rejectIndex));

     // delete later
     static int deleteLaterIndex = colorDialog->metaObject()->indexOfSlot("deleteLater()");
     QObject::connect(colorDialog, selectedColorSignal.method(), colorDialog, colorDialog->metaObject()->method(deleteLaterIndex));
     QObject::connect(colorDialog, rejectedSignal.method(), colorDialog, colorDialog->metaObject()->method(deleteLaterIndex));

    colorDialogComponent->completeCreate();
    QMetaObject::invokeMethod(colorDialog, "open");
}

void UIDelegatesManager::showDialog(QSharedPointer<AuthenticationDialogController> dialogController)
{
    Q_ASSERT(!dialogController.isNull());

    if (!ensureComponentLoaded(AuthenticationDialog)) {
        // Let the controller know it couldn't be loaded
        qWarning("Failed to load authentication dialog, rejecting.");
        dialogController->reject();
        return;
    }

    QQmlContext *context = qmlContext(m_view);
    QObject *authenticationDialog = authenticationDialogComponent->beginCreate(context);
    // set visual parent for non-Window-based dialogs
    if (QQuickItem* item = qobject_cast<QQuickItem*>(authenticationDialog))
        item->setParentItem(m_view);
    authenticationDialog->setParent(m_view);

    QString introMessage;
    if (dialogController->isProxy()) {
        introMessage = tr("Connect to proxy \"%1\" using:");
        introMessage = introMessage.arg(dialogController->host().toHtmlEscaped());
    } else {
        const QUrl url = dialogController->url();
        introMessage = tr("Enter username and password for \"%1\" at %2://%3");
        introMessage = introMessage.arg(dialogController->realm(), url.scheme(), url.host());
    }
    QQmlProperty textProp(authenticationDialog, QStringLiteral("text"));
    textProp.write(introMessage);

    QQmlProperty acceptSignal(authenticationDialog, QStringLiteral("onAccepted"));
    QQmlProperty rejectSignal(authenticationDialog, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(acceptSignal, authenticationDialogComponent->url());
    CHECK_QML_SIGNAL_PROPERTY(rejectSignal, authenticationDialogComponent->url());

    static int acceptIndex = dialogController->metaObject()->indexOfSlot("accept(QString,QString)");
    QObject::connect(authenticationDialog, acceptSignal.method(), dialogController.data(), dialogController->metaObject()->method(acceptIndex));
    static int rejectIndex = dialogController->metaObject()->indexOfSlot("reject()");
    QObject::connect(authenticationDialog, rejectSignal.method(), dialogController.data(), dialogController->metaObject()->method(rejectIndex));

    authenticationDialogComponent->completeCreate();
    QMetaObject::invokeMethod(authenticationDialog, "open");
}

void UIDelegatesManager::showFilePicker(FilePickerController *controller)
{

    if (!ensureComponentLoaded(FilePicker))
        return;

    QQmlContext *context = qmlContext(m_view);
    QObject *filePicker = filePickerComponent->beginCreate(context);
    if (QQuickItem* item = qobject_cast<QQuickItem*>(filePicker))
        item->setParentItem(m_view);
    filePicker->setParent(m_view);
    filePickerComponent->completeCreate();

    // Fine-tune some properties depending on the mode.
    switch (controller->mode()) {
    case FilePickerController::Open:
        break;
    case FilePickerController::Save:
        filePicker->setProperty("selectExisting", false);
        break;
    case FilePickerController::OpenMultiple:
        filePicker->setProperty("selectMultiple", true);
        break;
    case FilePickerController::UploadFolder:
        filePicker->setProperty("selectFolder", true);
        break;
    default:
        Q_UNREACHABLE();
    }

    controller->setParent(filePicker);

    QQmlProperty filesPickedSignal(filePicker, QStringLiteral("onFilesSelected"));
    CHECK_QML_SIGNAL_PROPERTY(filesPickedSignal, filePickerComponent->url());
    QQmlProperty rejectSignal(filePicker, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(rejectSignal, filePickerComponent->url());
    static int acceptedIndex = controller->metaObject()->indexOfSlot("accepted(QVariant)");
    QObject::connect(filePicker, filesPickedSignal.method(), controller, controller->metaObject()->method(acceptedIndex));
    static int rejectedIndex = controller->metaObject()->indexOfSlot("rejected()");
    QObject::connect(filePicker, rejectSignal.method(), controller, controller->metaObject()->method(rejectedIndex));

    // delete when done.
    static int deleteLaterIndex = filePicker->metaObject()->indexOfSlot("deleteLater()");
    QObject::connect(filePicker, filesPickedSignal.method(), filePicker, filePicker->metaObject()->method(deleteLaterIndex));
    QObject::connect(filePicker, rejectSignal.method(), filePicker, filePicker->metaObject()->method(deleteLaterIndex));

    QMetaObject::invokeMethod(filePicker, "open");
}

void UIDelegatesManager::showMessageBubble(const QRect &anchor, const QString &mainText, const QString &subText)
{
    if (!ensureComponentLoaded(MessageBubble))
        return;

    Q_ASSERT(m_messageBubbleItem.isNull());

    QQmlContext *context = qmlContext(m_view);
    m_messageBubbleItem.reset(qobject_cast<QQuickItem *>(messageBubbleComponent->beginCreate(context)));
    m_messageBubbleItem->setParentItem(m_view);
    messageBubbleComponent->completeCreate();

    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("maxWidth")).write(anchor.size().width());
    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("mainText")).write(mainText);
    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("subText")).write(subText);
    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("x")).write(anchor.x());
    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("y")).write(anchor.y() + anchor.size().height());
}

void UIDelegatesManager::hideMessageBubble()
{
    m_messageBubbleItem.reset();
}

void UIDelegatesManager::moveMessageBubble(const QRect &anchor)
{
    Q_ASSERT(!m_messageBubbleItem.isNull());

    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("x")).write(anchor.x());
    QQmlProperty(m_messageBubbleItem.data(), QStringLiteral("y")).write(anchor.y() + anchor.size().height());
}

} // namespace QtWebEngineCore
