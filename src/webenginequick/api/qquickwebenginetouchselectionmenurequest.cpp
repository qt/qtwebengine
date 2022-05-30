/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
