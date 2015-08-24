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

#include "qquickwebenginesettings_p.h"

#include "qquickwebengineprofile_p.h"
#include "web_engine_settings.h"

#include <QtCore/QList>

QT_BEGIN_NAMESPACE

using QtWebEngineCore::WebEngineSettings;

QQuickWebEngineSettings::QQuickWebEngineSettings(QQuickWebEngineSettings *parentSettings)
    : d_ptr(new WebEngineSettings(parentSettings ? parentSettings->d_ptr.data() : 0))
{ }

/*!
    \qmltype WebEngineSettings
    \instantiates QQuickWebEngineSettings
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.1
    \brief WebEngineSettings allows configuration of browser properties and attributes.

    WebEngineSettings allows configuration of browser properties and generic attributes, such as
    JavaScript support, focus behavior, and access to remote content.

    Each WebEngineView can have individual settings.

*/


QQuickWebEngineSettings::~QQuickWebEngineSettings()
{ }

/*!
    \qmlproperty bool WebEngineSettings::autoLoadImages

    Automatically loads images on web pages.

    Enabled by default.
*/
bool QQuickWebEngineSettings::autoLoadImages() const
{
    return d_ptr->testAttribute(WebEngineSettings::AutoLoadImages);
}

/*!
    \qmlproperty bool WebEngineSettings::javascriptEnabled

    Enables the running of JavaScript programs.

    Enabled by default.
*/
bool QQuickWebEngineSettings::javascriptEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::JavascriptEnabled);
}

/*!
    \qmlproperty bool WebEngineSettings::javascriptCanOpenWindows

    Allows JavaScript programs to open new windows.

    Enabled by default.
*/
bool QQuickWebEngineSettings::javascriptCanOpenWindows() const
{
    return d_ptr->testAttribute(WebEngineSettings::JavascriptCanOpenWindows);
}

/*!
    \qmlproperty bool WebEngineSettings::javascriptCanAccessClipboard

    Allows JavaScript programs to read from or write to the clipboard.

    Disabled by default.
*/
bool QQuickWebEngineSettings::javascriptCanAccessClipboard() const
{
    return d_ptr->testAttribute(WebEngineSettings::JavascriptCanAccessClipboard);
}

/*!
    \qmlproperty bool WebEngineSettings::linksIncludedInFocusChain

    Includes hyperlinks in the keyboard focus chain.

    Enabled by default.
*/
bool QQuickWebEngineSettings::linksIncludedInFocusChain() const
{
    return d_ptr->testAttribute(WebEngineSettings::LinksIncludedInFocusChain);
}

/*!
    \qmlproperty bool WebEngineSettings::localStorageEnabled

    Enables support for the HTML 5 local storage feature.

    Enabled by default.
*/
bool QQuickWebEngineSettings::localStorageEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::LocalStorageEnabled);
}

/*!
    \qmlproperty bool WebEngineSettings::localContentCanAccessRemoteUrls

    Allows locally loaded documents to access remote URLs.

    Disabled by default.
*/
bool QQuickWebEngineSettings::localContentCanAccessRemoteUrls() const
{
    return d_ptr->testAttribute(WebEngineSettings::LocalContentCanAccessRemoteUrls);
}

/*!
    \qmlproperty bool WebEngineSettings::spatialNavigationEnabled

    Enables the Spatial Navigation feature, which means the ability to navigate between focusable
    elements, such as hyperlinks and form controls, on a web page by using the Left, Right, Up and
    Down arrow keys.

    For example, if a user presses the Right key, heuristics determine whether there is an element
    they might be trying to reach towards the right and which element they probably want.

    Disabled by default.

*/
bool QQuickWebEngineSettings::spatialNavigationEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::SpatialNavigationEnabled);
}

/*!
    \qmlproperty bool WebEngineSettings::localContentCanAccessFileUrls

    Allows locally loaded documents to access other local URLs.

    Enabled by default.
*/
bool QQuickWebEngineSettings::localContentCanAccessFileUrls() const
{
    return d_ptr->testAttribute(WebEngineSettings::LocalContentCanAccessFileUrls);
}

/*!
    \qmlproperty bool WebEngineSettings::hyperlinkAuditingEnabled

    Enables support for the \c ping attribute for hyperlinks.

    Disabled by default.
*/
bool QQuickWebEngineSettings::hyperlinkAuditingEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::HyperlinkAuditingEnabled);
}

/*!
    \qmlproperty bool WebEngineSettings::errorPageEnabled

    Enables displaying the built-in error pages of Chromium.

    Enabled by default.
*/
bool QQuickWebEngineSettings::errorPageEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::ErrorPageEnabled);
}

/*!
    \qmlproperty QString WebEngineSettings::defaultTextEncoding

    Sets the default encoding. The value must be a string describing an encoding such as "utf-8" or
    "iso-8859-1".

    If left empty, a default value will be used.
*/
QString QQuickWebEngineSettings::defaultTextEncoding() const
{
    return d_ptr->defaultTextEncoding();
}

void QQuickWebEngineSettings::setAutoLoadImages(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::AutoLoadImages);
    // Set unconditionally as it sets the override for the current settings while the current setting
    // could be from the fallback and is prone to changing later on.
    d_ptr->setAttribute(WebEngineSettings::AutoLoadImages, on);
    if (wasOn != on)
        Q_EMIT autoLoadImagesChanged();
}

void QQuickWebEngineSettings::setJavascriptEnabled(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::JavascriptEnabled);
    d_ptr->setAttribute(WebEngineSettings::JavascriptEnabled, on);
    if (wasOn != on)
        Q_EMIT javascriptEnabledChanged();
}

void QQuickWebEngineSettings::setJavascriptCanOpenWindows(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::JavascriptCanOpenWindows);
    d_ptr->setAttribute(WebEngineSettings::JavascriptCanOpenWindows, on);
    if (wasOn != on)
        Q_EMIT javascriptCanOpenWindowsChanged();
}

void QQuickWebEngineSettings::setJavascriptCanAccessClipboard(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::JavascriptCanAccessClipboard);
    d_ptr->setAttribute(WebEngineSettings::JavascriptCanAccessClipboard, on);
    if (wasOn != on)
        Q_EMIT javascriptCanAccessClipboardChanged();
}

void QQuickWebEngineSettings::setLinksIncludedInFocusChain(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::LinksIncludedInFocusChain);
    d_ptr->setAttribute(WebEngineSettings::LinksIncludedInFocusChain, on);
    if (wasOn != on)
        Q_EMIT linksIncludedInFocusChainChanged();
}

void QQuickWebEngineSettings::setLocalStorageEnabled(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::LocalStorageEnabled);
    d_ptr->setAttribute(WebEngineSettings::LocalStorageEnabled, on);
    if (wasOn != on)
        Q_EMIT localStorageEnabledChanged();
}

void QQuickWebEngineSettings::setLocalContentCanAccessRemoteUrls(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::LocalContentCanAccessRemoteUrls);
    d_ptr->setAttribute(WebEngineSettings::LocalContentCanAccessRemoteUrls, on);
    if (wasOn != on)
        Q_EMIT localContentCanAccessRemoteUrlsChanged();
}


void QQuickWebEngineSettings::setSpatialNavigationEnabled(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::SpatialNavigationEnabled);
    d_ptr->setAttribute(WebEngineSettings::SpatialNavigationEnabled, on);
    if (wasOn != on)
        Q_EMIT spatialNavigationEnabledChanged();
}

void QQuickWebEngineSettings::setLocalContentCanAccessFileUrls(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::LocalContentCanAccessFileUrls);
    d_ptr->setAttribute(WebEngineSettings::LocalContentCanAccessFileUrls, on);
    if (wasOn != on)
        Q_EMIT localContentCanAccessFileUrlsChanged();
}

void QQuickWebEngineSettings::setHyperlinkAuditingEnabled(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::HyperlinkAuditingEnabled);
    d_ptr->setAttribute(WebEngineSettings::HyperlinkAuditingEnabled, on);
    if (wasOn != on)
        Q_EMIT hyperlinkAuditingEnabledChanged();
}

void QQuickWebEngineSettings::setErrorPageEnabled(bool on)
{
    bool wasOn = d_ptr->testAttribute(WebEngineSettings::ErrorPageEnabled);
    d_ptr->setAttribute(WebEngineSettings::ErrorPageEnabled, on);
    if (wasOn != on)
        Q_EMIT errorPageEnabledChanged();
}

void QQuickWebEngineSettings::setDefaultTextEncoding(QString encoding)
{
    const QString oldDefaultTextEncoding = d_ptr->defaultTextEncoding();
    d_ptr->setDefaultTextEncoding(encoding);
    if (oldDefaultTextEncoding.compare(encoding))
        Q_EMIT defaultTextEncodingChanged();
}

void QQuickWebEngineSettings::setParentSettings(QQuickWebEngineSettings *parentSettings)
{
    d_ptr->setParentSettings(parentSettings->d_ptr.data());
    d_ptr->scheduleApplyRecursively();
}

QT_END_NAMESPACE
