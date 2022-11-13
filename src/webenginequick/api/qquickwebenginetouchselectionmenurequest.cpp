// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginetouchselectionmenurequest_p.h"
#include "qquickwebenginetouchselectionmenurequest_p_p.h"
#include "touch_selection_menu_controller.h"

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(QQuickWebEngineTouchSelectionMenuRequest::Cut,
                   QtWebEngineCore::TouchSelectionMenuController::TouchSelectionCommandFlag::Cut)
ASSERT_ENUMS_MATCH(QQuickWebEngineTouchSelectionMenuRequest::Copy,
                   QtWebEngineCore::TouchSelectionMenuController::TouchSelectionCommandFlag::Copy)
ASSERT_ENUMS_MATCH(QQuickWebEngineTouchSelectionMenuRequest::Paste,
                   QtWebEngineCore::TouchSelectionMenuController::TouchSelectionCommandFlag::Paste)

/*!
    \class QQuickWebEngineTouchSelectionMenuRequest
    \since 6.3
    \brief The QQuickWebEngineTouchSelectionMenuRequest class provides request for a touch selection menu.
    \inmodule QtWebEngineQuick
    QQuickWebEngineTouchSelectionMenuRequest is returned after a touch selection event,
    and contains information about where the bounding box of touch selection is and what
    actions are available.

    \internal
*/

/*!
    \enum QQuickWebEngineTouchSelectionMenuRequest::TouchSelectionCommandFlag
    \readonly
    \since 6.3

    The available operations in the current touch selection menu request.

    \value Cut Cut is available.
    \value Copy Copy is available.
    \value Paste Paste is available.
*/
QQuickWebEngineTouchSelectionMenuRequest::QQuickWebEngineTouchSelectionMenuRequest(
        QRect bounds, QtWebEngineCore::TouchSelectionMenuController *touchSelectionMenuController)
    : d(new QQuickWebEngineTouchSelectionMenuRequestPrivate(bounds, touchSelectionMenuController))
{
}

QQuickWebEngineTouchSelectionMenuRequestPrivate::QQuickWebEngineTouchSelectionMenuRequestPrivate(
        QRect bounds, QtWebEngineCore::TouchSelectionMenuController *touchSelectionMenuController)
{
    selectionBounds = bounds;
    buttonCount = touchSelectionMenuController->buttonCount();
    touchSelectionCommandFlags = touchSelectionMenuController->availableActions();
}

/*!
    Destroys the touch selection menu request.
*/
QQuickWebEngineTouchSelectionMenuRequest::~QQuickWebEngineTouchSelectionMenuRequest()
{
}

/*!
    Returns the number of buttons that must be displayed, based on the available actions.
*/
int QQuickWebEngineTouchSelectionMenuRequest::buttonCount()
{
    return d->buttonCount;
}

/*!
    Indicates whether the touch selection menu request has been
    accepted by the signal handler.

    If the property is \c false after any signal handlers
    for WebEngineView::touchSelectionMenuRequested have been executed,
    a default touch selection menu will be shown.
    To prevent this, set \c{request.accepted} to \c true.

    The default is \c false.
*/
bool QQuickWebEngineTouchSelectionMenuRequest::isAccepted() const
{
    return d->accepted;
}

void QQuickWebEngineTouchSelectionMenuRequest::setAccepted(bool accepted)
{
    d->accepted = accepted;
}

/*!
    Returns the bound rectangle of text selection.
*/
QRect QQuickWebEngineTouchSelectionMenuRequest::selectionBounds()
{
    return d->selectionBounds;
}

/*!
    Returns the available operations in the current context.
*/
QQuickWebEngineTouchSelectionMenuRequest::TouchSelectionCommandFlags QQuickWebEngineTouchSelectionMenuRequest::touchSelectionCommandFlags() const
{
    return static_cast<QQuickWebEngineTouchSelectionMenuRequest::TouchSelectionCommandFlags>(d->touchSelectionCommandFlags);
}

QT_END_NAMESPACE

#include "moc_qquickwebenginetouchselectionmenurequest_p.cpp"
