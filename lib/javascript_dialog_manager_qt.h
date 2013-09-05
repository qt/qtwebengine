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
#ifndef JAVASCRIPT_DIALOG_MANAGER_QT_H
#define JAVASCRIPT_DIALOG_MANAGER_QT_H

#include "content/public/browser/javascript_dialog_manager.h"

#include "content/public/common/javascript_message_type.h"

namespace content {
class WebContents;
}
class WebContentsAdapterClient;

class JavaScriptDialogManagerQt : public content::JavaScriptDialogManager {

public:
    // For use with the Singleton helper class from chromium
    static JavaScriptDialogManagerQt* GetInstance();

    // Displays a JavaScript dialog. |did_suppress_message| will not be nil; if
      // |true| is returned in it, the caller will handle faking the reply.
      virtual void RunJavaScriptDialog(
          content::WebContents* web_contents,
          const GURL& origin_url,
          const std::string& accept_lang,
          content::JavaScriptMessageType javascript_message_type,
          const base::string16& message_text,
          const base::string16& default_prompt_text,
          const content::JavaScriptDialogManager::DialogClosedCallback& callback,
          bool* did_suppress_message);

      // Displays a dialog asking the user if they want to leave a page.
      virtual void RunBeforeUnloadDialog(content::WebContents* web_contents,
                                         const base::string16& message_text,
                                         bool is_reload,
                                         const content::JavaScriptDialogManager::DialogClosedCallback& callback) {}

      // Accepts or dismisses the active JavaScript dialog, which must be owned
      // by the given |web_contents|. If |prompt_override| is not null, the prompt
      // text of the dialog should be set before accepting. Returns true if the
      // dialog was handled.
      virtual bool HandleJavaScriptDialog(content::WebContents* web_contents,
                                          bool accept,
                                          const base::string16* prompt_override);

      // Cancels all active and pending dialogs for the given WebContents.
    virtual void CancelActiveAndPendingDialogs(content::WebContents* web_contents) {};

      // The given WebContents is being destroyed; discards any saved state tied
      // to it.
    virtual void WebContentsDestroyed(content::WebContents* web_contents) {};

};


#endif // JAVASCRIPT_DIALOG_MANAGER_QT_H

