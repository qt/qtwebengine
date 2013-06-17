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
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents.h"

#include <QObject>


namespace content {
    class BrowserContext;
    class SiteInstance;
}

class WebContentsDelegateQt : public QObject
                            , public content::WebContentsDelegate
                            , public content::NotificationObserver
{
    Q_OBJECT
public:
    WebContentsDelegateQt(QObject* webContentsView, content::BrowserContext*, content::SiteInstance*, int routing_id, const gfx::Size& initial_size);
    content::WebContents* web_contents();

    virtual void Observe(int type, const content::NotificationSource&, const content::NotificationDetails&);
    virtual void NavigationStateChanged(const content::WebContents* source, unsigned changed_flags);
    virtual void LoadingStateChanged(content::WebContents* source);

Q_SIGNALS:
    void titleChanged(QString title);
    void urlChanged();
    void loadingStateChanged();

private:
    scoped_ptr<content::WebContents> m_webContents;
    content::NotificationRegistrar m_registrar;
    QObject* m_webContentsView;
};

#endif
