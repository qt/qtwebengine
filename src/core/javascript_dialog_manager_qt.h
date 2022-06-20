// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef JAVASCRIPT_DIALOG_MANAGER_QT_H
#define JAVASCRIPT_DIALOG_MANAGER_QT_H

#include "content/public/browser/javascript_dialog_manager.h"

#include "web_contents_adapter_client.h"

#include <QMap>
#include <QSharedPointer>

namespace content {
class WebContents;
}

namespace QtWebEngineCore {
class JavaScriptDialogController;

class JavaScriptDialogManagerQt : public content::JavaScriptDialogManager
{
public:
    // For use with the Singleton helper class from chromium
    static JavaScriptDialogManagerQt *GetInstance();

    void RunJavaScriptDialog(content::WebContents *, content::RenderFrameHost *, content::JavaScriptDialogType dialog_type,
                             const std::u16string &messageText, const std::u16string &defaultPromptText,
                             DialogClosedCallback callback,
                             bool *didSuppressMessage) override;
    void RunBeforeUnloadDialog(content::WebContents *web_contents,
                               content::RenderFrameHost *render_frame_host,
                               bool is_reload,
                               DialogClosedCallback callback) override;
    bool HandleJavaScriptDialog(content::WebContents *, bool accept, const std::u16string *promptOverride) override;
    void CancelDialogs(content::WebContents *contents, bool /*reset_state*/) override
    {
        takeDialogForContents(contents);
    }

    void runDialogForContents(content::WebContents *, WebContentsAdapterClient::JavascriptDialogType, const QString &messageText, const QString &defaultPrompt
                              , const QUrl &, DialogClosedCallback &&callback, const QString &title = QString());
    QSharedPointer<JavaScriptDialogController> takeDialogForContents(content::WebContents *);

private:
    QMap<content::WebContents *, QSharedPointer<JavaScriptDialogController> > m_activeDialogs;

};

} // namespace QtWebEngineCore

#endif // JAVASCRIPT_DIALOG_MANAGER_QT_H

