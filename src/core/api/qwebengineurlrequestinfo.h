// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLREQUESTINFO_H
#define QWEBENGINEURLREQUESTINFO_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qurl.h>

#include <memory>

namespace QtWebEngineCore {
class ContentBrowserClientQt;
class InterceptedRequest;
} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE

class QWebEngineUrlRequestInfoPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineUrlRequestInfo
{
public:
    enum ResourceType {
        ResourceTypeMainFrame = 0,  // top level page
        ResourceTypeSubFrame,       // frame or iframe
        ResourceTypeStylesheet,     // a CSS stylesheet
        ResourceTypeScript,         // an external script
        ResourceTypeImage,          // an image (jpg/gif/png/etc)
        ResourceTypeFontResource,   // a font
        ResourceTypeSubResource,    // an "other" subresource.
        ResourceTypeObject,         // an object (or embed) tag for a plugin,
                                    // or a resource that a plugin requested.
        ResourceTypeMedia,          // a media resource.
        ResourceTypeWorker,         // the main resource of a dedicated worker.
        ResourceTypeSharedWorker,   // the main resource of a shared worker.
        ResourceTypePrefetch,       // an explicitly requested prefetch
        ResourceTypeFavicon,        // a favicon
        ResourceTypeXhr,            // a XMLHttpRequest
        ResourceTypePing,           // a ping request for <a ping>
        ResourceTypeServiceWorker,  // the main resource of a service worker.
        ResourceTypeCspReport,      // Content Security Policy (CSP) violation report
        ResourceTypePluginResource, // A resource requested by a plugin
        ResourceTypeNavigationPreloadMainFrame = 19, // A main-frame service worker navigation preload request
        ResourceTypeNavigationPreloadSubFrame,  // A sub-frame service worker navigation preload request
#ifndef Q_QDOC
        ResourceTypeLast = ResourceTypeNavigationPreloadSubFrame,
#endif
        ResourceTypeWebSocket = 254,
        ResourceTypeUnknown = 255
    };

    enum NavigationType {
        NavigationTypeLink,
        NavigationTypeTyped,
        NavigationTypeFormSubmitted,
        NavigationTypeBackForward,
        NavigationTypeReload,
        NavigationTypeOther,
        NavigationTypeRedirect,
    };

    ResourceType resourceType() const;
    NavigationType navigationType() const;

    QUrl requestUrl() const;
    QUrl firstPartyUrl() const;
    QUrl initiator() const;
    QByteArray requestMethod() const;
    bool changed() const;

    void block(bool shouldBlock);
    void redirect(const QUrl &url);
    void setHttpHeader(const QByteArray &name, const QByteArray &value);
    QHash<QByteArray, QByteArray> httpHeaders() const;

private:
    friend class QtWebEngineCore::ContentBrowserClientQt;
    friend class QtWebEngineCore::InterceptedRequest;
    Q_DISABLE_COPY(QWebEngineUrlRequestInfo)
    Q_DECLARE_PRIVATE(QWebEngineUrlRequestInfo)

    void resetChanged();

    QWebEngineUrlRequestInfo();
    QWebEngineUrlRequestInfo(QWebEngineUrlRequestInfoPrivate *p);
    QWebEngineUrlRequestInfo(QWebEngineUrlRequestInfo &&p);
    QWebEngineUrlRequestInfo &operator=(QWebEngineUrlRequestInfo &&p);
    ~QWebEngineUrlRequestInfo();
    std::unique_ptr<QWebEngineUrlRequestInfoPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLREQUESTINFO_H
