// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "javascript_dialog_manager_qt.h"

#include "javascript_dialog_controller.h"
#include "javascript_dialog_controller_p.h"
#include "web_contents_view_qt.h"
#include "type_conversion.h"


#include "base/memory/singleton.h"
#include "content/browser/web_contents/web_contents_impl.h"

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(content::JAVASCRIPT_DIALOG_TYPE_ALERT, WebContentsAdapterClient::AlertDialog)
ASSERT_ENUMS_MATCH(content::JAVASCRIPT_DIALOG_TYPE_CONFIRM, WebContentsAdapterClient::ConfirmDialog)
ASSERT_ENUMS_MATCH(content::JAVASCRIPT_DIALOG_TYPE_PROMPT, WebContentsAdapterClient::PromptDialog)


JavaScriptDialogManagerQt *JavaScriptDialogManagerQt::GetInstance()
{
    return base::Singleton<JavaScriptDialogManagerQt>::get();
}

void JavaScriptDialogManagerQt::RunJavaScriptDialog(content::WebContents *webContents,
                                                    content::RenderFrameHost *renderFrameHost,
                                                    content::JavaScriptDialogType dialog_type,
                                                    const std::u16string &messageText,
                                                    const std::u16string &defaultPromptText,
                                                    content::JavaScriptDialogManager::DialogClosedCallback callback,
                                                    bool *didSuppressMessage)
{
    WebContentsAdapterClient *client = WebContentsViewQt::from(static_cast<content::WebContentsImpl*>(webContents)->GetView())->client();
    if (!client) {
        if (didSuppressMessage)
            *didSuppressMessage = true;
        return;
    }
    const GURL originUrl = renderFrameHost->GetLastCommittedOrigin().GetURL();
    WebContentsAdapterClient::JavascriptDialogType dialogType = static_cast<WebContentsAdapterClient::JavascriptDialogType>(dialog_type);
    runDialogForContents(webContents, dialogType, toQt(messageText), toQt(defaultPromptText), toQt(originUrl), std::move(callback));
}

void JavaScriptDialogManagerQt::RunBeforeUnloadDialog(content::WebContents *webContents, content::RenderFrameHost *renderFrameHost,
                                                      bool isReload,
                                                      content::JavaScriptDialogManager::DialogClosedCallback callback) {
    Q_UNUSED(isReload);
    const GURL originUrl = renderFrameHost->GetLastCommittedOrigin().GetURL();
    runDialogForContents(webContents, WebContentsAdapterClient::UnloadDialog, QString(), QString(), toQt(originUrl), std::move(callback));
}

bool JavaScriptDialogManagerQt::HandleJavaScriptDialog(content::WebContents *contents, bool accept, const std::u16string *promptOverride)
{
    QSharedPointer<JavaScriptDialogController> dialog = m_activeDialogs.value(contents);
    if (!dialog)
        return false;
    dialog->d->dialogFinished(accept, promptOverride ? *promptOverride : std::u16string());
    takeDialogForContents(contents);
    return true;
}

void JavaScriptDialogManagerQt::runDialogForContents(content::WebContents *webContents, WebContentsAdapterClient::JavascriptDialogType type
                                                     , const QString &messageText, const QString &defaultPrompt, const QUrl &origin
                                                     , content::JavaScriptDialogManager::DialogClosedCallback &&callback, const QString &title)
{
    WebContentsAdapterClient *client = WebContentsViewQt::from(static_cast<content::WebContentsImpl*>(webContents)->GetView())->client();
    if (!client)
        return;

    JavaScriptDialogControllerPrivate *dialogData = new JavaScriptDialogControllerPrivate(type, messageText, defaultPrompt, title, origin, std::move(callback), webContents);
    QSharedPointer<JavaScriptDialogController> dialog(new JavaScriptDialogController(dialogData));

    // We shouldn't get new dialogs for a given WebContents until we gave back a result.
    Q_ASSERT(!m_activeDialogs.contains(webContents));
    m_activeDialogs.insert(webContents, dialog);

    client->javascriptDialog(dialog);

}

QSharedPointer<JavaScriptDialogController> JavaScriptDialogManagerQt::takeDialogForContents(content::WebContents *contents)
{
    QSharedPointer<JavaScriptDialogController> dialog = m_activeDialogs.take(contents);
    if (dialog)
        Q_EMIT dialog->dialogCloseRequested();
    return dialog;
}

} // namespace QtWebEngineCore
