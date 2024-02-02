// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef JAVASCRIPT_DIALOG_CONTROLLER_P_H
#define JAVASCRIPT_DIALOG_CONTROLLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "base/functional/callback.h"
#include "content/public/browser/javascript_dialog_manager.h"

#include "web_contents_adapter_client.h"

#include <QString>
#include <QUrl>

namespace content {
class WebContents;
}

namespace QtWebEngineCore {

class JavaScriptDialogControllerPrivate {

public:
    void dialogFinished(bool accepted, const std::u16string &promptValue);
    JavaScriptDialogControllerPrivate(WebContentsAdapterClient::JavascriptDialogType, const QString &message, const QString &prompt
                                      , const QString& title, const QUrl &securityOrigin
                                      , content::JavaScriptDialogManager::DialogClosedCallback &&, content::WebContents *);

    WebContentsAdapterClient::JavascriptDialogType type;
    QString message;
    QString defaultPrompt;
    QUrl securityOrigin;
    QString userInput;
    QString title;
    content::JavaScriptDialogManager::DialogClosedCallback callback;
    content::WebContents *contents;
};

} // namespace QtWebEngineCore

#endif // JAVASCRIPT_DIALOG_CONTROLLER_P_H
