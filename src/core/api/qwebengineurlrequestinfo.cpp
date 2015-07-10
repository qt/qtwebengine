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

#include "qwebengineurlrequestinfo.h"
#include "qwebengineurlrequestinfo_p.h"

#include "content/public/common/resource_type.h"

#include "web_contents_adapter_client.h"

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeMainFrame, content::RESOURCE_TYPE_MAIN_FRAME)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeSubFrame, content::RESOURCE_TYPE_SUB_FRAME)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeStylesheet, content::RESOURCE_TYPE_STYLESHEET)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeScript, content::RESOURCE_TYPE_SCRIPT)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeImage, content::RESOURCE_TYPE_IMAGE)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeFontResource, content::RESOURCE_TYPE_FONT_RESOURCE)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeSubResource, content::RESOURCE_TYPE_SUB_RESOURCE)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeObject, content::RESOURCE_TYPE_OBJECT)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeMedia, content::RESOURCE_TYPE_MEDIA)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeWorker, content::RESOURCE_TYPE_WORKER)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeSharedWorker, content::RESOURCE_TYPE_SHARED_WORKER)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypePrefetch, content::RESOURCE_TYPE_PREFETCH)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeFavicon, content::RESOURCE_TYPE_FAVICON)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeXhr, content::RESOURCE_TYPE_XHR)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypePing, content::RESOURCE_TYPE_PING)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeServiceWorker, content::RESOURCE_TYPE_SERVICE_WORKER)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeUnknown, content::RESOURCE_TYPE_LAST_TYPE)

ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::LinkNavigation, QWebEngineUrlRequestInfo::NavigationTypeLink)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::TypedNavigation, QWebEngineUrlRequestInfo::NavigationTypeTyped)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::FormSubmittedNavigation, QWebEngineUrlRequestInfo::NavigationTypeFormSubmitted)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::BackForwardNavigation, QWebEngineUrlRequestInfo::NavigationTypeBackForward)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::ReloadNavigation, QWebEngineUrlRequestInfo::NavigationTypeReload)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::OtherNavigation, QWebEngineUrlRequestInfo::NavigationTypeOther)

QWebEngineUrlRequestInfoPrivate::QWebEngineUrlRequestInfoPrivate(QWebEngineUrlRequestInfo::ResourceType resource, QWebEngineUrlRequestInfo::NavigationType navigation, const QUrl &u, const QByteArray &m)
    : resourceType(resource)
    , navigationType(navigation)
    , shouldBlockRequest(false)
    , url(u)
    , method(m)
{
}

QWebEngineUrlRequestInfo::~QWebEngineUrlRequestInfo()
{

}

QWebEngineUrlRequestInfo::QWebEngineUrlRequestInfo(QWebEngineUrlRequestInfoPrivate *p)
    : d_ptr(p)
{
    d_ptr->q_ptr = this;
}

QWebEngineUrlRequestInfo::ResourceType QWebEngineUrlRequestInfo::resourceType() const
{
    Q_D(const QWebEngineUrlRequestInfo);
    return d->resourceType;
}

QWebEngineUrlRequestInfo::NavigationType QWebEngineUrlRequestInfo::navigationType() const
{
    Q_D(const QWebEngineUrlRequestInfo);
    return d->navigationType;
}

const QUrl &QWebEngineUrlRequestInfo::url() const
{
    Q_D(const QWebEngineUrlRequestInfo);
    return d->url;
}

const QByteArray &QWebEngineUrlRequestInfo::method() const
{
    Q_D(const QWebEngineUrlRequestInfo);
    return d->method;
}

void QWebEngineUrlRequestInfo::redirectTo(const QUrl &url)
{
    Q_D(QWebEngineUrlRequestInfo);
    d->url = url;
}

void QWebEngineUrlRequestInfo::blockRequest(bool shouldBlock)
{
    Q_D(QWebEngineUrlRequestInfo);
    d->shouldBlockRequest = shouldBlock;
}

void QWebEngineUrlRequestInfo::setExtraHeader(const QByteArray &name, const QByteArray &value)
{
    Q_D(QWebEngineUrlRequestInfo);
    d->extraHeaders.insert(name, value);
}

QT_END_NAMESPACE
