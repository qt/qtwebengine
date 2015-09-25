/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

#include "content/public/browser/javascript_dialog_manager.h"
#include "web_contents_adapter_client.h"
#include <QString>

namespace content {
class WebContents;
}

namespace QtWebEngineCore {

class JavaScriptDialogControllerPrivate {

public:
    void dialogFinished(bool accepted, const base::string16 &promptValue);
    JavaScriptDialogControllerPrivate(WebContentsAdapterClient::JavascriptDialogType, const QString &message, const QString &prompt
                                      , const QString& title, const QUrl &securityOrigin
                                      , const content::JavaScriptDialogManager::DialogClosedCallback &, content::WebContents *);

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
