// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "touchselectionmenuwidget_p.h"
#include "qwebengineview.h"

#include "touch_selection_menu_controller.h"

#include <QBoxLayout>
#include <QEvent>

namespace QtWebEngineWidgetUI {
namespace {
// The Widgets module is built with rtti in developer-build while Core is not.
// The MOC will throw "undefined reference to typeinfo..." errors if we connect slots
// from QtWebEngineCore via function pointers, because it expects rtti on them.
// To workaround this we use lambdas.
void connectButton(TouchSelectionMenuWidget::TouchButton *button, std::function<void()> callback)
{
    QObject::connect(button, &QPushButton::clicked, std::move(callback));
}
} // namespace

TouchSelectionMenuWidget::TouchButton::TouchButton(QString name, QWidget *parent)
    : QPushButton(name, parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
}

TouchSelectionMenuWidget::TouchButton::~TouchButton()
{
}

bool TouchSelectionMenuWidget::TouchButton::event(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::TouchBegin:
        ev->accept();
        return true;
    case QEvent::TouchEnd:
        Q_EMIT clicked();
        ev->accept();
        return true;
    default:
        break;
    }

    return QPushButton::event(ev);
}

TouchSelectionMenuWidget::TouchSelectionMenuWidget(
        QWebEngineView *view, QtWebEngineCore::TouchSelectionMenuController *controller)
    : QWidget(view,
              Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
              | Qt::WindowDoesNotAcceptFocus)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_DeleteOnClose, true);

    bool cutEnabled =
            controller->isCommandEnabled(QtWebEngineCore::TouchSelectionMenuController::Cut);
    bool copyEnabled =
            controller->isCommandEnabled(QtWebEngineCore::TouchSelectionMenuController::Copy);
    bool pasteEnabled =
            controller->isCommandEnabled(QtWebEngineCore::TouchSelectionMenuController::Paste);

    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *layout = new QHBoxLayout();

    if (cutEnabled) {
        TouchButton *button = new TouchButton(tr("Cut"), this);
        button->setSizePolicy(policy);
        layout->addWidget(button);
        connectButton(button, [controller]() { controller->cut(); });
    }

    if (copyEnabled) {
        TouchButton *button = new TouchButton(tr("Copy"), this);
        button->setSizePolicy(policy);
        layout->addWidget(button);
        connectButton(button, [controller]() { controller->copy(); });
    }

    if (pasteEnabled) {
        TouchButton *button = new TouchButton(tr("Paste"), this);
        button->setSizePolicy(policy);
        layout->addWidget(button);
        connectButton(button, [controller]() { controller->paste(); });
    }

    TouchButton *button = new TouchButton(tr("..."), this);
    button->setSizePolicy(policy);
    layout->addWidget(button);
    connectButton(button, [controller]() { controller->runContextMenu(); });

    layout->setSpacing(2);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    nativeParentWidget()->installEventFilter(this);
}

TouchSelectionMenuWidget::~TouchSelectionMenuWidget()
{
}

bool TouchSelectionMenuWidget::eventFilter(QObject *obj, QEvent *ev)
{
    // Close the menu if the window is moved
    if (ev->type() == QEvent::Move)
        close();

    return QWidget::eventFilter(obj, ev);
}
} // QtWebEngineWidgetUI
