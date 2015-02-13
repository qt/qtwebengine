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

QQuickWebEngineSettings::QQuickWebEngineSettings(QQuickWebEngineSettings *parentSettings)
    : d_ptr(new WebEngineSettings(parentSettings ? parentSettings->d_ptr.data() : 0))
{ }

QQuickWebEngineSettings::~QQuickWebEngineSettings()
{ }

bool QQuickWebEngineSettings::autoLoadImages() const
{
    return d_ptr->testAttribute(WebEngineSettings::AutoLoadImages);
}

bool QQuickWebEngineSettings::javascriptEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::JavascriptEnabled);
}

bool QQuickWebEngineSettings::javascriptCanOpenWindows() const
{
    return d_ptr->testAttribute(WebEngineSettings::JavascriptCanOpenWindows);
}

bool QQuickWebEngineSettings::javascriptCanAccessClipboard() const
{
    return d_ptr->testAttribute(WebEngineSettings::JavascriptCanAccessClipboard);
}

bool QQuickWebEngineSettings::linksIncludedInFocusChain() const
{
    return d_ptr->testAttribute(WebEngineSettings::LinksIncludedInFocusChain);
}

bool QQuickWebEngineSettings::localStorageEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::LocalStorageEnabled);
}

bool QQuickWebEngineSettings::localContentCanAccessRemoteUrls() const
{
    return d_ptr->testAttribute(WebEngineSettings::LocalContentCanAccessRemoteUrls);
}

bool QQuickWebEngineSettings::spatialNavigationEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::SpatialNavigationEnabled);
}

bool QQuickWebEngineSettings::localContentCanAccessFileUrls() const
{
    return d_ptr->testAttribute(WebEngineSettings::LocalContentCanAccessFileUrls);
}

bool QQuickWebEngineSettings::hyperlinkAuditingEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::HyperlinkAuditingEnabled);
}

bool QQuickWebEngineSettings::errorPageEnabled() const
{
    return d_ptr->testAttribute(WebEngineSettings::ErrorPageEnabled);
}

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
