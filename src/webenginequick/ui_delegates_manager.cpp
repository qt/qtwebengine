// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "ui_delegates_manager.h"

#include "api/qquickwebengineaction_p.h"
#include "api/qquickwebengineview_p_p.h"

#include <authentication_dialog_controller.h>
#include <autofill_popup_controller.h>
#include <color_chooser_controller.h>
#include <file_picker_controller.h>
#include <javascript_dialog_controller.h>
#include <touch_selection_menu_controller.h>
#include <web_contents_adapter_client.h>

#include <QtCore/qdiriterator.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlist.h>
#include <QtCore/qtimer.h>
#include <QtGui/qcursor.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>
#include <QtQuick/qquickwindow.h>

#include <algorithm>

// Uncomment for QML debugging
//#define UI_DELEGATES_DEBUG

namespace QtWebEngineCore {

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

static QPoint calculateToolTipPosition(QPoint &position, QSize &toolTip) {
    QRect screen;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (const QScreen *src : screens)
        if (src->availableGeometry().contains(position))
            screen = src->availableGeometry();

    position += QPoint(2, 16);

    if (position.x() + toolTip.width() > screen.x() + screen.width())
        position.rx() -= 4 + toolTip.width();
    if (position.y() + toolTip.height() > screen.y() + screen.height())
        position.ry() -= 24 + toolTip.height();
    if (position.y() < screen.y())
        position.setY(screen.y());
    if (position.x() + toolTip.width() > screen.x() + screen.width())
        position.setX(screen.x() + screen.width() - toolTip.width());
    if (position.x() < screen.x())
        position.setX(screen.x());
    if (position.y() + toolTip.height() > screen.y() + screen.height())
        position.setY(screen.y() + screen.height() - toolTip.height());

    return position;
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

#define COMPONENT_MEMBER_INIT(TYPE, COMPONENT) \
    , COMPONENT##Component(0)

// clang-format off
UIDelegatesManager::UIDelegatesManager(QQuickWebEngineView *view)
    : m_view(view)
    , m_toolTip(nullptr)
    , m_touchSelectionMenu(nullptr)
    , m_autofillPopup(nullptr)
    FOR_EACH_COMPONENT_TYPE(COMPONENT_MEMBER_INIT, NO_SEPARATOR)
// clang-format on
{
}

UIDelegatesManager::~UIDelegatesManager()
{
}

#define COMPONENT_MEMBER_CASE_STATEMENT(TYPE, COMPONENT) \
    case TYPE: \
        component = &COMPONENT##Component; \
        break;

bool UIDelegatesManager::ensureComponentLoaded(ComponentType type)
{
    QQmlEngine* engine = qmlEngine(m_view);
    if (m_importDirs.isEmpty() && !initializeImportDirs(m_importDirs, engine))
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

    for (const QString &importDir : std::as_const(m_importDirs)) {
        const QString componentFilePath = importDir % QLatin1Char('/') % fileName;

        if (!QFileInfo(componentFilePath).exists())
            continue;

        // FIXME: handle async loading
        *component = (new QQmlComponent(engine,
                                        importDir.startsWith(QLatin1String(":/")) ? QUrl(QLatin1String("qrc") + componentFilePath)
                                                                                  : QUrl::fromLocalFile(componentFilePath),
                                        QQmlComponent::PreferSynchronous, m_view));

        if ((*component)->status() != QQmlComponent::Ready) {
            const QList<QQmlError> errs = (*component)->errors();
            for (const QQmlError &err : errs)
                qWarning("QtWebEngine: component error: %s\n", qPrintable(err.toString()));
            delete *component;
            *component = nullptr;
            return false;
        }
        return true;
    }
    return false;
}

#define CHECK_QML_SIGNAL_PROPERTY(prop, location) \
    if (!prop.isSignalProperty()) \
        qWarning("%s is missing %s signal property.\n", qPrintable(location.toString()), qPrintable(prop.name()));

void UIDelegatesManager::addMenuSeparator(QObject *menu)
{
    if (!ensureComponentLoaded(MenuSeparator))
        return;

    QQmlContext *itemContext = qmlContext(m_view);
    QObject *sep = menuSeparatorComponent->create(itemContext);
    sep->setParent(menu);

    QQmlListReference entries(menu, defaultPropertyName(menu));
    if (entries.isValid() && entries.count() > 0)
        entries.append(sep);
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

    QQmlComponent *dialogComponent = nullptr;
    switch (dialogComponentType) {
    FOR_EACH_COMPONENT_TYPE(ASSIGN_DIALOG_COMPONENT_DATA_CASE_STATEMENT, NO_SEPARATOR)
    default:
        Q_UNREACHABLE();
    }

    QQmlContext *context = qmlContext(m_view);
    QObject *dialog = dialogComponent->beginCreate(context);
    // set visual parent for non-Window-based dialogs
    if (QQuickItem *item = qobject_cast<QQuickItem*>(dialog))
        item->setParentItem(m_view);
    dialog->setParent(m_view);
    QQmlProperty textProp(dialog, QStringLiteral("text"));
    if (dialogController->type() == WebContentsAdapterClient::UnloadDialog)
        textProp.write(tr("Changes that you made may not be saved."));
    else
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
    if (QQuickItem *item = qobject_cast<QQuickItem*>(colorDialog))
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
    if (QQuickItem *item = qobject_cast<QQuickItem*>(authenticationDialog))
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
    static int deleteLaterIndex = authenticationDialog->metaObject()->indexOfSlot("deleteLater()");
    QObject::connect(authenticationDialog, acceptSignal.method(), dialogController.data(), dialogController->metaObject()->method(acceptIndex));
    QObject::connect(authenticationDialog, acceptSignal.method(), authenticationDialog, authenticationDialog->metaObject()->method(deleteLaterIndex));
    static int rejectIndex = dialogController->metaObject()->indexOfSlot("reject()");
    QObject::connect(authenticationDialog, rejectSignal.method(), dialogController.data(), dialogController->metaObject()->method(rejectIndex));
    QObject::connect(authenticationDialog, rejectSignal.method(), authenticationDialog, authenticationDialog->metaObject()->method(deleteLaterIndex));

    authenticationDialogComponent->completeCreate();
    QMetaObject::invokeMethod(authenticationDialog, "open");
}

void UIDelegatesManager::showFilePicker(QSharedPointer<FilePickerController> controller)
{
    if (controller->mode() == FilePickerController::UploadFolder) {
        showDirectoryPicker(controller);
        return;
    }

    if (!ensureComponentLoaded(FilePicker))
        return;

    QQmlContext *context = qmlContext(m_view);
    QObject *filePicker = filePickerComponent->beginCreate(context);
    if (QQuickItem *item = qobject_cast<QQuickItem*>(filePicker))
        item->setParentItem(m_view);
    filePicker->setParent(m_view);
    filePickerComponent->completeCreate();

    static int fileModeIndex = filePicker->metaObject()->indexOfEnumerator("FileMode");
    QMetaEnum fileModeEnum = filePicker->metaObject()->enumerator(fileModeIndex);

    // Fine-tune some properties depending on the mode.
    switch (controller->mode()) {
    case FilePickerController::Open:
        filePicker->setProperty("fileMode", fileModeEnum.keyToValue("OpenFile"));
        break;
    case FilePickerController::Save:
        filePicker->setProperty("fileMode", fileModeEnum.keyToValue("SaveFile"));
        break;
    case FilePickerController::OpenMultiple:
        filePicker->setProperty("fileMode", fileModeEnum.keyToValue("OpenFiles"));
        break;
    case FilePickerController::UploadFolder:
    default:
        Q_UNREACHABLE();
    }

    filePicker->setProperty("nameFilters", FilePickerController::nameFilters(controller->acceptedMimeTypes()));

    QQmlProperty filesPickedSignal(filePicker, QStringLiteral("onFilesSelected"));
    CHECK_QML_SIGNAL_PROPERTY(filesPickedSignal, filePickerComponent->url());
    QQmlProperty rejectSignal(filePicker, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(rejectSignal, filePickerComponent->url());
    static int acceptedIndex = controller->metaObject()->indexOfSlot("accepted(QVariant)");
    QObject::connect(filePicker, filesPickedSignal.method(), controller.data(), controller->metaObject()->method(acceptedIndex));
    static int rejectedIndex = controller->metaObject()->indexOfSlot("rejected()");
    QObject::connect(filePicker, rejectSignal.method(), controller.data(), controller->metaObject()->method(rejectedIndex));

    // delete when done.
    static int deleteLaterIndex = filePicker->metaObject()->indexOfSlot("deleteLater()");
    QObject::connect(filePicker, filesPickedSignal.method(), filePicker, filePicker->metaObject()->method(deleteLaterIndex));
    QObject::connect(filePicker, rejectSignal.method(), filePicker, filePicker->metaObject()->method(deleteLaterIndex));

    QMetaObject::invokeMethod(filePicker, "open");
}

void UIDelegatesManager::showDirectoryPicker(QSharedPointer<FilePickerController> controller)
{
    if (!ensureComponentLoaded(DirectoryPicker))
        return;

    QQmlContext *context = qmlContext(m_view);
    QObject *directoryPicker = directoryPickerComponent->beginCreate(context);
    if (QQuickItem *item = qobject_cast<QQuickItem*>(directoryPicker))
        item->setParentItem(m_view);
    directoryPicker->setParent(m_view);
    directoryPickerComponent->completeCreate();

    QQmlProperty directoryPickedSignal(directoryPicker, QStringLiteral("onFolderSelected"));
    CHECK_QML_SIGNAL_PROPERTY(directoryPickedSignal, directoryPickerComponent->url());
    QQmlProperty rejectSignal(directoryPicker, QStringLiteral("onRejected"));
    CHECK_QML_SIGNAL_PROPERTY(rejectSignal, directoryPickerComponent->url());
    static int acceptedIndex = controller->metaObject()->indexOfSlot("accepted(QVariant)");
    QObject::connect(directoryPicker, directoryPickedSignal.method(), controller.data(), controller->metaObject()->method(acceptedIndex));
    static int rejectedIndex = controller->metaObject()->indexOfSlot("rejected()");
    QObject::connect(directoryPicker, rejectSignal.method(), controller.data(), controller->metaObject()->method(rejectedIndex));

    // delete when done.
    static int deleteLaterIndex = directoryPicker->metaObject()->indexOfSlot("deleteLater()");
    QObject::connect(directoryPicker, directoryPickedSignal.method(), directoryPicker, directoryPicker->metaObject()->method(deleteLaterIndex));
    QObject::connect(directoryPicker, rejectSignal.method(), directoryPicker, directoryPicker->metaObject()->method(deleteLaterIndex));

    QMetaObject::invokeMethod(directoryPicker, "open");
}

class TemporaryCursorMove
{
public:
    TemporaryCursorMove(const QQuickItem *item, const QPoint &pos)
    {
        if (pos.isNull() || !item->contains(pos))
            return;
        const QPoint oldPos = QCursor::pos();
        const QPoint globalPos = item->mapToGlobal(QPointF(pos)).toPoint();
        if (oldPos == globalPos)
            return;
        m_oldCursorPos = oldPos;
        QCursor::setPos(globalPos);
    }

    ~TemporaryCursorMove()
    {
        if (!m_oldCursorPos.isNull())
            QCursor::setPos(m_oldCursorPos);
    }

private:
    QPoint m_oldCursorPos;
};

void UIDelegatesManager::showToolTip(const QString &text)
{
    if (text.isEmpty()) {
        m_toolTip.reset();
        return;
    }

    if (!ensureComponentLoaded(ToolTip))
        return;

    if (!m_toolTip.isNull())
        return;

    QQmlContext *context = qmlContext(m_view);
    m_toolTip.reset(toolTipComponent->beginCreate(context));
    if (QQuickItem *item = qobject_cast<QQuickItem *>(m_toolTip.data()))
        item->setParentItem(m_view);
    m_toolTip->setParent(m_view);
    toolTipComponent->completeCreate();

    QQmlProperty(m_toolTip.data(), QStringLiteral("text")).write(text);

    int height = QQmlProperty(m_toolTip.data(), QStringLiteral("height")).read().toInt();
    int width = QQmlProperty(m_toolTip.data(), QStringLiteral("width")).read().toInt();
    QSize toolTipSize(width, height);
    QPoint position = m_view->cursor().pos();
    position = m_view->mapFromGlobal(calculateToolTipPosition(position, toolTipSize)).toPoint();

    QQmlProperty(m_toolTip.data(), QStringLiteral("x")).write(position.x());
    QQmlProperty(m_toolTip.data(), QStringLiteral("y")).write(position.y());

    QMetaObject::invokeMethod(m_toolTip.data(), "open");
}

QQuickItem *UIDelegatesManager::createTouchHandle()
{
    if (!ensureComponentLoaded(TouchHandle))
        return nullptr;

    QQmlContext *context = qmlContext(m_view);
    QObject *touchHandle = touchHandleComponent->beginCreate(context);
    QQuickItem *item = qobject_cast<QQuickItem *>(touchHandle);
    Q_ASSERT(item);
    item->setParentItem(m_view);
    touchHandleComponent->completeCreate();

    return item;
}

void UIDelegatesManager::showTouchSelectionMenu(QtWebEngineCore::TouchSelectionMenuController *menuController, const QRect &bounds, const int spacing)
{
    if (!ensureComponentLoaded(TouchSelectionMenu))
        return;

    QQmlContext *context = qmlContext(m_view);
    m_touchSelectionMenu.reset(touchSelectionMenuComponent->beginCreate(context));
    if (QQuickItem *item = qobject_cast<QQuickItem *>(m_touchSelectionMenu.data()))
        item->setParentItem(m_view);
    m_touchSelectionMenu->setParent(m_view);

    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("width")).write(bounds.width());
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("height")).write(bounds.height());
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("x")).write(bounds.x());
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("y")).write(bounds.y());
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("border.width")).write(spacing);

    // Cut button
    bool cutEnabled = menuController->isCommandEnabled(TouchSelectionMenuController::Cut);
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("isCutEnabled")).write(cutEnabled);
    if (cutEnabled) {
        QQmlProperty cutSignal(m_touchSelectionMenu.data(), QStringLiteral("onCutTriggered"));
        CHECK_QML_SIGNAL_PROPERTY(cutSignal, touchSelectionMenuComponent->url());
        int cutIndex = menuController->metaObject()->indexOfSlot("cut()");
        QObject::connect(m_touchSelectionMenu.data(), cutSignal.method(), menuController, menuController->metaObject()->method(cutIndex));
    }

    // Copy button
    bool copyEnabled = menuController->isCommandEnabled(TouchSelectionMenuController::Copy);
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("isCopyEnabled")).write(copyEnabled);
    if (copyEnabled) {
        QQmlProperty copySignal(m_touchSelectionMenu.data(), QStringLiteral("onCopyTriggered"));
        CHECK_QML_SIGNAL_PROPERTY(copySignal, touchSelectionMenuComponent->url());
        int copyIndex = menuController->metaObject()->indexOfSlot("copy()");
        QObject::connect(m_touchSelectionMenu.data(), copySignal.method(), menuController, menuController->metaObject()->method(copyIndex));
    }

    // Paste button
    bool pasteEnabled = menuController->isCommandEnabled(TouchSelectionMenuController::Paste);
    QQmlProperty(m_touchSelectionMenu.data(), QStringLiteral("isPasteEnabled")).write(pasteEnabled);
    if (pasteEnabled) {
        QQmlProperty pasteSignal(m_touchSelectionMenu.data(), QStringLiteral("onPasteTriggered"));
        CHECK_QML_SIGNAL_PROPERTY(pasteSignal, touchSelectionMenuComponent->url());
        int pasteIndex = menuController->metaObject()->indexOfSlot("paste()");
        QObject::connect(m_touchSelectionMenu.data(), pasteSignal.method(), menuController, menuController->metaObject()->method(pasteIndex));
    }

    // Context menu button
    QQmlProperty contextMenuSignal(m_touchSelectionMenu.data(), QStringLiteral("onContextMenuTriggered"));
    CHECK_QML_SIGNAL_PROPERTY(contextMenuSignal, touchSelectionMenuComponent->url());
    int contextMenuIndex = menuController->metaObject()->indexOfSlot("runContextMenu()");
    QObject::connect(m_touchSelectionMenu.data(), contextMenuSignal.method(), menuController, menuController->metaObject()->method(contextMenuIndex));

    touchSelectionMenuComponent->completeCreate();
}

void UIDelegatesManager::hideTouchSelectionMenu()
{
    QTimer::singleShot(0, m_view, [this] { m_touchSelectionMenu.reset(); });
}

bool AutofillPopupEventFilter::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            m_manager->hideAutofillPopup();
            return true;
        }

        // Ignore shortcuts while the popup is open. It may result unwanted
        // edit commands sent to Chromium that blocks the key press.
        event->ignore();
        return true;
    }

    // AutofillPopupControllerImpl::HandleKeyPressEvent()
    // chrome/browser/ui/autofill/autofill_popup_controller_impl.cc

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up:
            m_controller->selectPreviousSuggestion();
            return true;
        case Qt::Key_Down:
            m_controller->selectNextSuggestion();
            return true;
        case Qt::Key_PageUp:
            m_controller->selectFirstSuggestion();
            return true;
        case Qt::Key_PageDown:
            m_controller->selectLastSuggestion();
            return true;
        case Qt::Key_Escape:
            m_manager->hideAutofillPopup();
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            m_controller->acceptSuggestion();
            return true;
        case Qt::Key_Delete:
            // Remove suggestion is not supported for datalist.
            // Forward delete to view to be able to remove selected text.
            break;
        case Qt::Key_Tab:
            m_controller->acceptSuggestion();
            break;
        default:
            break;
        }
    }

    // Do not forward release events of the overridden key presses.
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Escape:
        case Qt::Key_Enter:
        case Qt::Key_Return:
            return true;
        default:
            break;
        }
    }

    return QObject::eventFilter(object, event);
}

void UIDelegatesManager::showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller,
                                           QPointF pos, int width, bool autoselectFirstSuggestion)
{
    static const int padding = 1;
    static const int itemHeight = 20;
    const int proposedHeight = itemHeight * (controller->model()->rowCount()) + padding * 2;

    bool popupWasNull = false;
    if (m_autofillPopup.isNull()) {
        popupWasNull = true;
        if (!ensureComponentLoaded(AutofillPopup))
            return;

        QQmlContext *context = qmlContext(m_view);
        m_autofillPopup.reset(autofillPopupComponent->beginCreate(context));
        if (QQuickItem *item = qobject_cast<QQuickItem *>(m_autofillPopup.data()))
            item->setParentItem(m_view);
        m_autofillPopup->setParent(m_view);
    }

    m_autofillPopup->setProperty("controller", QVariant::fromValue(controller));
    m_autofillPopup->setProperty("x", pos.x());
    m_autofillPopup->setProperty("y", pos.y());
    m_autofillPopup->setProperty("width", width);
    m_autofillPopup->setProperty("height",
                                 std::min(proposedHeight, qRound(m_view->height() - pos.y())));
    m_autofillPopup->setProperty("padding", padding);
    m_autofillPopup->setProperty("itemHeight", itemHeight);

    if (popupWasNull) {
        QQmlProperty selectedSignal(m_autofillPopup.data(), QStringLiteral("onSelected"));
        CHECK_QML_SIGNAL_PROPERTY(selectedSignal, autofillPopupComponent->url());
        static int selectSuggestionIndex =
                controller->metaObject()->indexOfSlot("selectSuggestion(int)");
        QObject::connect(m_autofillPopup.data(), selectedSignal.method(), controller,
                         controller->metaObject()->method(selectSuggestionIndex));

        QQmlProperty acceptedSignal(m_autofillPopup.data(), QStringLiteral("onAccepted"));
        CHECK_QML_SIGNAL_PROPERTY(acceptedSignal, autofillPopupComponent->url());
        static int acceptSuggestionIndex =
                controller->metaObject()->indexOfSlot("acceptSuggestion()");
        QObject::connect(m_autofillPopup.data(), acceptedSignal.method(), controller,
                         controller->metaObject()->method(acceptSuggestionIndex));

        QObject::connect(controller, &QtWebEngineCore::AutofillPopupController::currentIndexChanged,
                         [this](const QModelIndex &index) {
                             QMetaObject::invokeMethod(m_autofillPopup.data(), "setCurrentIndex",
                                                       Qt::DirectConnection,
                                                       Q_ARG(QVariant, index.row()));
                         });

        autofillPopupComponent->completeCreate();

        m_view->window()->installEventFilter(
                new AutofillPopupEventFilter(controller, this, m_autofillPopup.data()));

        QMetaObject::invokeMethod(m_autofillPopup.data(), "open");
        controller->notifyPopupShown();
    }

    if (autoselectFirstSuggestion)
        controller->selectFirstSuggestion();
}

void UIDelegatesManager::hideAutofillPopup()
{
    if (!m_autofillPopup)
        return;

    QTimer::singleShot(0, m_view, [this] {
        if (m_autofillPopup) {
            QtWebEngineCore::AutofillPopupController *controller =
                    m_autofillPopup->property("controller")
                            .value<QtWebEngineCore::AutofillPopupController *>();
            m_autofillPopup.reset();
            controller->notifyPopupHidden();
        }
    });
}

bool UIDelegatesManager::initializeImportDirs(QStringList &dirs, QQmlEngine *engine)
{
    const QStringList paths = engine->importPathList();
    for (const QString &path : paths) {
        QString controlsImportPath = path % QLatin1String("/QtWebEngine/ControlsDelegates/");

        // resource paths have to be tested using the ":/" prefix
        if (controlsImportPath.startsWith(QLatin1String("qrc:/"))) {
            controlsImportPath.remove(0, 3);
        }

        QFileInfo fi(controlsImportPath);
        if (fi.exists()) {
            dirs << fi.absolutePath();

            // add subdirectories
            QDirIterator it(controlsImportPath, QDir::AllDirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext())
                dirs << QFileInfo(it.next()).absoluteFilePath();
        }
    }
    return !dirs.isEmpty();
}

QObject *UIDelegatesManager::addMenu(QObject *parentMenu, const QString &title, const QPoint &pos)
{
    Q_ASSERT(parentMenu);
    if (!ensureComponentLoaded(Menu))
        return nullptr;
    QQmlContext *context = qmlContext(m_view);
    QObject *menu = menuComponent->beginCreate(context);

    // set visual parent for non-Window-based menus
    if (QQuickItem *item = qobject_cast<QQuickItem*>(menu))
        item->setParentItem(m_view);

    if (!title.isEmpty())
        menu->setProperty("title", title);
    if (!pos.isNull()) {
        menu->setProperty("x", pos.x());
        menu->setProperty("y", pos.y());
    }

    menu->setParent(parentMenu);
    QQmlProperty doneSignal(menu, QStringLiteral("onDone"));
    CHECK_QML_SIGNAL_PROPERTY(doneSignal, menuComponent->url())
    static int deleteLaterIndex = menu->metaObject()->indexOfSlot("deleteLater()");
    QObject::connect(menu, doneSignal.method(), menu, menu->metaObject()->method(deleteLaterIndex));
    menuComponent->completeCreate();
    return menu;
}

void UIDelegatesManager::addMenuItem(QQuickWebEngineAction *action, QObject *menu, bool checkable, bool checked)
{
    Q_ASSERT(action);
    if (!ensureComponentLoaded(MenuItem))
        return;

    QObject *it = menuItemComponent->beginCreate(qmlContext(m_view));

    it->setProperty("text", action->text());
    it->setProperty("enabled", action->isEnabled());
    it->setProperty("checked", checked);
    it->setProperty("checkable", checkable);

    QQmlProperty signal(it, QStringLiteral("onTriggered"));
    CHECK_QML_SIGNAL_PROPERTY(signal, menuItemComponent->url());
    const QMetaObject *actionMeta = action->metaObject();
    QObject::connect(it, signal.method(), action, actionMeta->method(actionMeta->indexOfSlot("trigger()")));
    menuItemComponent->completeCreate();

    it->setParent(menu);

    QQmlListReference entries(menu, defaultPropertyName(menu));
    if (entries.isValid())
        entries.append(it);
}

void UIDelegatesManager::showMenu(QObject *menu)
{
    QMetaObject::invokeMethod(menu, "open");
}

} // namespace QtWebEngineCore
