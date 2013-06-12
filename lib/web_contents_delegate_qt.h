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

#ifndef WEB_CONTENTS_DELEGATE_QT
#define WEB_CONTENTS_DELEGATE_QT

#include "content/public/browser/notification_observer.h"
#include "content/public/browser/web_contents_delegate.h"


namespace content {
    class BrowserContext;
    class SiteInstance;
}

class QWebContentsView;
class QQuickWebContentsView;

class WebContentsDelegateQt : public content::WebContentsDelegate
                            , public content::NotificationObserver
{
public:
    static WebContentsDelegateQt* CreateNewWindow(content::BrowserContext*, const GURL&, content::SiteInstance*, int routing_id, const gfx::Size& initial_size);
    content::WebContents* web_contents();
    void LoadURL(const GURL&);
    void LoadURLForFrame(const GURL&, const std::string& frame_name);
    void GoBackOrForward(int offset);
    void Reload();
    void Stop();

    virtual void Observe(int, const content::NotificationSource&, const content::NotificationDetails&);

private:
    WebContentsDelegateQt(content::WebContents*);

    scoped_ptr<content::WebContents> m_webContents;

    static std::vector<WebContentsDelegateQt*> m_windows;
};

#endif
