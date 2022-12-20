// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef UI_DELEGATES_MANAGER_H
#define UI_DELEGATES_MANAGER_H

#include <QtCore/qcoreapplication.h> // Q_DECLARE_TR_FUNCTIONS
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

// clang-format off
#define FOR_EACH_COMPONENT_TYPE(F, SEPARATOR) \
    F(Menu, menu) SEPARATOR \
    F(MenuItem, menuItem) SEPARATOR \
    F(MenuSeparator, menuSeparator) SEPARATOR \
    F(AlertDialog, alertDialog) SEPARATOR \
    F(ColorDialog, colorDialog) SEPARATOR \
    F(ConfirmDialog, confirmDialog) SEPARATOR \
    F(PromptDialog, promptDialog) SEPARATOR \
    F(FilePicker, filePicker) SEPARATOR \
    F(DirectoryPicker, directoryPicker) SEPARATOR \
    F(AuthenticationDialog, authenticationDialog) SEPARATOR \
    F(ToolTip, toolTip) SEPARATOR \
    F(TouchHandle, touchHandle) SEPARATOR \
    F(TouchSelectionMenu, touchSelectionMenu) SEPARATOR \
    F(AutofillPopup, autofillPopup) SEPARATOR

#define COMMA_SEPARATOR ,
#define SEMICOLON_SEPARATOR ;
#define ENUM_DECLARATION(TYPE, COMPONENT) \
    TYPE
#define MEMBER_DECLARATION(TYPE, COMPONENT) \
    QQmlComponent *COMPONENT##Component
// clang-format on

QT_BEGIN_NAMESPACE
class QEvent;
class QQmlComponent;
class QQmlContext;
class QQmlEngine;
class QQuickItem;
class QQuickWebEngineAction;
class QQuickWebEngineView;
QT_END_NAMESPACE

namespace QtWebEngineCore {
class AuthenticationDialogController;
class AutofillPopupController;
class ColorChooserController;
class FilePickerController;
class JavaScriptDialogController;
class TouchSelectionMenuController;

const char *defaultPropertyName(QObject *obj);

class UIDelegatesManager
{
    Q_DECLARE_TR_FUNCTIONS(UIDelegatesManager)
public:
    enum ComponentType {
        Invalid = -1,
        FOR_EACH_COMPONENT_TYPE(ENUM_DECLARATION, COMMA_SEPARATOR)
        ComponentTypeCount
    };

    UIDelegatesManager(QQuickWebEngineView *);
    virtual ~UIDelegatesManager();

    virtual bool initializeImportDirs(QStringList &dirs, QQmlEngine *engine);
    virtual void addMenuItem(QQuickWebEngineAction *action, QObject *menu,
                             bool checkable = false, bool checked = true);
    void addMenuSeparator(QObject *menu);
    virtual QObject *addMenu(QObject *parentMenu, const QString &title,
                             const QPoint &pos = QPoint());
    QQmlContext *creationContextForComponent(QQmlComponent *);
    void showColorDialog(QSharedPointer<ColorChooserController>);
    void showDialog(QSharedPointer<JavaScriptDialogController>);
    void showDialog(QSharedPointer<AuthenticationDialogController>);
    void showFilePicker(QSharedPointer<FilePickerController>);
    void showDirectoryPicker(QSharedPointer<FilePickerController>);
    virtual void showMenu(QObject *menu);
    void showToolTip(const QString &text);
    QQuickItem *createTouchHandle();
    void showTouchSelectionMenu(TouchSelectionMenuController *, const QRect &, const int spacing);
    void hideTouchSelectionMenu();
    void showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller, QPointF pos,
                           int width, bool autoselectFirstSuggestion);
    void hideAutofillPopup();

private:
    bool ensureComponentLoaded(ComponentType);

    QQuickWebEngineView *m_view;
    QStringList m_importDirs;
    QScopedPointer<QObject> m_toolTip;
    QScopedPointer<QObject> m_touchSelectionMenu;
    QScopedPointer<QObject> m_autofillPopup;

    FOR_EACH_COMPONENT_TYPE(MEMBER_DECLARATION, SEMICOLON_SEPARATOR)

    Q_DISABLE_COPY(UIDelegatesManager)

};

class AutofillPopupEventFilter : public QObject
{
    Q_OBJECT

public:
    AutofillPopupEventFilter(QtWebEngineCore::AutofillPopupController *controller,
                             UIDelegatesManager *manager, QObject *parent)
        : QObject(parent), m_controller(controller), m_manager(manager)
    {
    }

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QtWebEngineCore::AutofillPopupController *m_controller;
    UIDelegatesManager *m_manager;
};

} // namespace QtWebEngineCore

#endif // UI_DELEGATES_MANAGER_H
