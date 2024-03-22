// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineframe.h"

#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineFrame
    \brief The QWebEngineFrame class gives information about and control over a page frame.
    \since 6.8

    \inmodule QtWebEngineCore

    A web engine frame represents a single frame within a web page, such as those created by
    \c <frame> or \c <iframe> HTML elements.
    An active QWebEnginePage has one or more frames arranged in a tree structure. The top-level
    frame, the root of this tree, can be accessed through the mainFrame() method, and
    children() provides a frame's direct descendants.

    A frame's lifetime is, at most, as long as the QWebEnginePage object that produced it.
    However, frames may be created and deleted spontaneously and dynamically, for example through
    navigation and script execution. Because of this, many QWebEngineFrame methods return
    optional values, which will be \c std::nullopt if the frame no longer exists.
*/

/*! \internal
 */
QWebEngineFrame::QWebEngineFrame(QtWebEngineCore::WebContentsAdapterClient *adapter, quint64 id)
    : m_adapterClient(adapter), m_id(id)
{
}

/*!
    Returns \c{true} if this object represents an existing frame; \c{false} otherwise.

    Once a frame is invalid, it never becomes valid again.
*/
bool QWebEngineFrame::isValid() const
{
    return m_adapterClient->webContentsAdapter()->hasFrame(m_id);
}

/*!
    Returns the frame name; that is, what would be returned by \c window.name in JavaScript.

    If the frame could not be found, returns a null QString.

    \sa htmlName
*/
QString QWebEngineFrame::name() const
{
    return m_adapterClient->webContentsAdapter()->frameName(m_id);
}

/*!
    Returns the value of the frame's \c name HTML attribute, or an empty string if it has none.

    If the frame could not be found, returns a null QString.

    \sa name
*/
QString QWebEngineFrame::htmlName() const
{
    return m_adapterClient->webContentsAdapter()->frameHtmlName(m_id);
}

/*!
    Returns a list of the frame's children in an arbitrary order.

    If the frame could not be found, returns an empty list.
 */
QList<QWebEngineFrame> QWebEngineFrame::children() const
{
    QList<QWebEngineFrame> result;
    for (auto childId : m_adapterClient->webContentsAdapter()->frameChildren(m_id))
        result.push_back(QWebEngineFrame{ m_adapterClient, childId });
    return result;
}

/*!
    Returns the URL of the content currently loaded in this frame.

    If the frame could not be found, returns an empty QUrl.
 */
QUrl QWebEngineFrame::url() const
{
    return m_adapterClient->webContentsAdapter()->frameUrl(m_id);
}

/*!
    Returns the size of the frame within the viewport.

    If the frame could not be found, returns QSizeF().
 */
QSizeF QWebEngineFrame::size() const
{
    return m_adapterClient->webContentsAdapter()->frameSize(m_id);
}

/*! \fn bool QWebEngineFrame::operator==(const QWebEngineFrame &left, const QWebEngineFrame &right) noexcept

    Returns \c{true} if \a left and \a right represent the same frame in the same web page,
   otherwise \c{false}.
 */

/*! \fn bool QWebEngineFrame::operator!=(const QWebEngineFrame &left, const QWebEngineFrame &right) noexcept

    Returns \c{true} if \a left and \a right represent different frames in the same web page,
   otherwise \c{false}.
 */

QT_END_NAMESPACE

#include "moc_qwebengineframe.cpp"
