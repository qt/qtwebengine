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

#include "javascript_dialog_controller.h"
#include "javascript_dialog_controller_p.h"

#include"javascript_dialog_manager_qt.h"
#include "type_conversion.h"

void JavaScriptDialogControllerPrivate::dialogFinished(bool accepted, const base::string16 &promptValue)
{
    // clear the queue first as this could result in the engine asking us to run another dialog.
    JavaScriptDialogManagerQt::GetInstance()->removeDialogForContents(contents);

    callback.Run(accepted, promptValue);
}

JavaScriptDialogControllerPrivate::JavaScriptDialogControllerPrivate(WebContentsAdapterClient::JavascriptDialogType t, const QString &msg, const QString &prompt
                                                                     , const content::JavaScriptDialogManager::DialogClosedCallback &cb, content::WebContents *c)
    : type(t)
    , message(msg)
    , defaultPrompt(prompt)
    , callback(cb)
    , contents(c)
{
}

JavaScriptDialogController::~JavaScriptDialogController()
{
}

QString JavaScriptDialogController::message() const
{
    return d->message;
}

QString JavaScriptDialogController::defaultPrompt() const
{
    return d->defaultPrompt;
}

WebContentsAdapterClient::JavascriptDialogType JavaScriptDialogController::type() const
{
    return d->type;
}

void JavaScriptDialogController::textProvided(const QString &text)
{
    d->userInput = text;
}

void JavaScriptDialogController::accept()
{
    d->dialogFinished(true, toString16(d->userInput));
}

void JavaScriptDialogController::reject()
{
    d->dialogFinished(false, toString16(d->defaultPrompt));
}

JavaScriptDialogController::JavaScriptDialogController(JavaScriptDialogControllerPrivate *dd)
{
    Q_ASSERT(dd);
    d.reset(dd);
}
