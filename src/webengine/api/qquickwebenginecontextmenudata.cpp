/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickwebenginecontextmenudata_p.h"

#include "web_contents_adapter_client.h"

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeNone,   QQuickWebEngineContextMenuData::MediaTypeNone)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeImage,  QQuickWebEngineContextMenuData::MediaTypeImage)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeAudio,  QQuickWebEngineContextMenuData::MediaTypeAudio)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeVideo,  QQuickWebEngineContextMenuData::MediaTypeVideo)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeCanvas, QQuickWebEngineContextMenuData::MediaTypeCanvas)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeFile,   QQuickWebEngineContextMenuData::MediaTypeFile)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypePlugin, QQuickWebEngineContextMenuData::MediaTypePlugin)

/*!
    \qmltype WebEngineContextMenuData
    \instantiates QQuickWebEngineContextMenuData
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.3
    \brief Provides context data for populating or extending a context menu with actions.


    WebEngineContextMenuData is returned by WebEngineView::contextMenuData() after a context menu event,
    and contains information about where the context menu event took place. This is also in the context
    in which any context specific WebEngineView::WebAction will be performed.
*/

QQuickWebEngineContextMenuData::QQuickWebEngineContextMenuData() : d(nullptr)
{
}

QQuickWebEngineContextMenuData::~QQuickWebEngineContextMenuData()
{
    delete d;
}

/*!
    \qmlproperty bool WebEngineDownloadItem::isValid

    Is \c true if the context data is valid; otherwise \c false.
*/
bool QQuickWebEngineContextMenuData::isValid() const
{
    return d;
}

/*!
    \qmlproperty QPoint WebEngineDownloadItem::position


    Returns the position of the context, usually the mouse position where the context menu event was triggered.
*/
QPoint QQuickWebEngineContextMenuData::position() const
{
    return d ? d->pos : QPoint();
}

/*!
    \qmlproperty QString WebEngineDownloadItem::linkText

    Returns the text of a link if the context is a link.
*/
QString QQuickWebEngineContextMenuData::linkText() const
{
    return d ? d->linkText : QString();
}

/*!
    \qmlproperty QUrl WebEngineDownloadItem::linkUrl

    Returns the URL of a link if the context is a link.
*/
QUrl QQuickWebEngineContextMenuData::linkUrl() const
{
    return d ? d->linkUrl : QUrl();
}

/*!
    \qmlproperty QString WebEngineDownloadItem::selectedText

    Returns the selected text of the context.
*/
QString QQuickWebEngineContextMenuData::selectedText() const
{
    return d ? d->selectedText : QString();
}

/*!
    \qmlproperty QUrl WebEngineDownloadItem::mediaUrl

    If the context is a media element, returns the URL of that media.
*/
QUrl QQuickWebEngineContextMenuData::mediaUrl() const
{
    return d ? d->mediaUrl : QUrl();
}

/*!
    \qmlproperty MediaType WebEngineDownloadItem::mediaType

    Returns the type of the media element or \c MediaTypeNone if the context is not a media element.

    \value  MediaTypeNone
            The context is not a media element.
    \value  MediaTypeImage
            The context is an image element
    \value  MediaTypeVideo
            The context is a video element
    \value  MediaTypeAudio
            The context is an audio element
    \value  MediaTypeCanvas
            The context is a canvas element
    \value  MediaTypeFile
            The context is a file
    \value  MediaTypePlugin
            The context is a plugin
*/

QQuickWebEngineContextMenuData::MediaType QQuickWebEngineContextMenuData::mediaType() const
{
    return d ? static_cast<QQuickWebEngineContextMenuData::MediaType>(d->mediaType) : MediaTypeNone;
}

/*!
    \qmlproperty bool WebEngineDownloadItem::isContentEditable

    Returns \c true if the content is editable by the user; otherwise returns \c false.
*/
bool QQuickWebEngineContextMenuData::isContentEditable() const
{
    return d ? d->isEditable : false;
}

void QQuickWebEngineContextMenuData::update(const QtWebEngineCore::WebEngineContextMenuData &update)
{
    const QQuickWebEngineContextMenuData old(d);
    d = new QtWebEngineCore::WebEngineContextMenuData(update);

    if (isValid() != old.isValid())
        Q_EMIT isValidChanged();

    if (position() != old.position())
        Q_EMIT positionChanged();

    if (selectedText() != old.selectedText())
        Q_EMIT selectedTextChanged();

    if (linkText() != old.linkText())
        Q_EMIT linkTextChanged();

    if (linkUrl() != old.linkUrl())
        Q_EMIT linkUrlChanged();

    if (mediaUrl() != old.mediaUrl())
        Q_EMIT mediaUrlChanged();

    if (mediaType() != old.mediaType())
        Q_EMIT mediaTypeChanged();

    if (isContentEditable() != old.isContentEditable())
        Q_EMIT isContentEditableChanged();
}

QQuickWebEngineContextMenuData::QQuickWebEngineContextMenuData(const QQuickWebEngineContextMenuDataPrivate *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

QT_END_NAMESPACE
