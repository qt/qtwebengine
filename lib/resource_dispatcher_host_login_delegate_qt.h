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


#ifndef RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_QT_H_
#define RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_QT_H_

#include "base/compiler_specific.h"
#include "base/strings/string16.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"

#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE


namespace net
{
    class AuthChallengeInfo;
    class URLRequest;
}

class WebContentsAdapterClient;

// This class provides a dialog box to ask the user for credentials. Useful in
// ResourceDispatcherHostDelegate::CreateLoginDelegate.
class ResourceDispatcherHostLoginDelegateQt: public content::ResourceDispatcherHostLoginDelegate
{
public:
    // Threading: IO thread.
    ResourceDispatcherHostLoginDelegateQt(net::AuthChallengeInfo* auth_info, net::URLRequest* request);

    // ResourceDispatcherHostLoginDelegate implementation:
    // Threading: IO thread.
    virtual void OnRequestCancelled() Q_DECL_OVERRIDE;

    // Called by the platform specific code when the user responds. Public because
    // the aforementioned platform specific code may not have access to private
    // members. Not to be called from client code.
    // Threading: UI thread.
    void UserAcceptedAuth(const string16& username, const string16& password);

    void UserCancelledAuth();

protected:
    // Threading: any
    virtual ~ResourceDispatcherHostLoginDelegateQt();

private:
    // All the methods that begin with Platform need to be implemented by the
    // platform specific LoginDialog implementation.
    // Creates the dialog.
    // Threading: UI thread.
    void PlatformCreateDialog(const string16& host, const string16& realm);

    // Called from the destructor to let each platform do any necessary cleanup.
    // Threading: UI thread.
    void PlatformCleanUp();

    // Called from OnRequestCancelled if the request was cancelled.
    // Threading: UI thread.
    void PlatformRequestCancelled();

    // Sets up dialog creation.
    // Threading: UI thread.
    void PrepDialog(const string16& host, const string16& realm);

    // Sends the authentication to the requester.
    // Threading: IO thread.
    void SendAuthToRequester(bool success, const string16& username, const string16& password);

private:
    // The request that wants login data.
    // Threading: IO thread.
    net::URLRequest* m_urlRequest;

    WebContentsAdapterClient *m_webContentsAdapterClient;
};


#endif  // RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_QT_H_
