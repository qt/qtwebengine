// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "javascript_dialog_controller.h"
#include "javascript_dialog_controller_p.h"

#include "javascript_dialog_manager_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

void JavaScriptDialogControllerPrivate::dialogFinished(bool accepted, const std::u16string &promptValue)
{
    // Clear the queue first as this could result in the engine asking us to run another dialog,
    // but hold a shared pointer so the dialog does not get deleted prematurely when running in-process.
    QSharedPointer<JavaScriptDialogController> dialog = JavaScriptDialogManagerQt::GetInstance()->takeDialogForContents(contents);

    std::move(callback).Run(accepted, promptValue);
}

JavaScriptDialogControllerPrivate::JavaScriptDialogControllerPrivate(WebContentsAdapterClient::JavascriptDialogType t, const QString &msg, const QString &prompt
                                                                     , const QString &title, const QUrl &securityOrigin
                                                                     , content::JavaScriptDialogManager::DialogClosedCallback &&cb, content::WebContents *c)
    : type(t)
    , message(msg)
    , defaultPrompt(prompt)
    , securityOrigin(securityOrigin)
    , title(title)
    , callback(std::move(cb))
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

QString JavaScriptDialogController::title() const
{
    return d->title;
}

WebContentsAdapterClient::JavascriptDialogType JavaScriptDialogController::type() const
{
    return d->type;
}

QUrl JavaScriptDialogController::securityOrigin() const
{
    return d->securityOrigin;
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

} // namespace QtWebEngineCore
