/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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

#include "qquickpdfnavigationstack_p.h"
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcNav, "qt.pdf.navigationstack")

/*!
    \qmltype PdfNavigationStack
    \instantiates QQuickPdfNavigationStack
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief History of the destinations visited within a PDF Document.
    \since 5.15

    PdfNavigationStack remembers which destinations the user has visited in a PDF
    document, and provides the ability to traverse backward and forward.
*/

QQuickPdfNavigationStack::QQuickPdfNavigationStack(QObject *parent)
    : QObject(parent)
{
    push(0, QPointF(), 1);
}

/*!
    \qmlmethod void PdfNavigationStack::forward()

    Goes back to the page, location and zoom level that was being viewed before
    back() was called, and then emits the \l jumped() signal.

    If a new destination was pushed since the last time \l back() was called,
    the forward() function does nothing, because there is a branch in the
    timeline which causes the "future" to be lost.
*/
void QQuickPdfNavigationStack::forward()
{
    if (m_currentHistoryIndex >= m_pageHistory.count() - 1)
        return;
    bool backAvailableWas = backAvailable();
    bool forwardAvailableWas = forwardAvailable();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    ++m_currentHistoryIndex;
    m_changing = true;
    emit jumped(currentPage(), currentLocation(), currentZoom());
    if (currentZoomWas != currentZoom())
        emit currentZoomChanged();
    emit currentPageChanged();
    if (currentLocationWas != currentLocation())
        emit currentLocationChanged();
    if (!backAvailableWas)
        emit backAvailableChanged();
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged();
    m_changing = false;
    qCDebug(qLcNav) << "forward: index" << m_currentHistoryIndex << "page" << currentPage()
                    << "@" << currentLocation() << "zoom" << currentZoom();
}

/*!
    \qmlmethod void PdfNavigationStack::back()

    Pops the stack, updates the \l currentPage, \l currentLocation and
    \l currentZoom properties to the most-recently-viewed destination, and then
    emits the \l jumped() signal.
*/
void QQuickPdfNavigationStack::back()
{
    if (m_currentHistoryIndex <= 0)
        return;
    bool backAvailableWas = backAvailable();
    bool forwardAvailableWas = forwardAvailable();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    --m_currentHistoryIndex;
    m_changing = true;
    emit jumped(currentPage(), currentLocation(), currentZoom());
    if (currentZoomWas != currentZoom())
        emit currentZoomChanged();
    emit currentPageChanged();
    if (currentLocationWas != currentLocation())
        emit currentLocationChanged();
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged();
    if (!forwardAvailableWas)
        emit forwardAvailableChanged();
    m_changing = false;
    qCDebug(qLcNav) << "back: index" << m_currentHistoryIndex << "page" << currentPage()
                    << "@" << currentLocation() << "zoom" << currentZoom();
}

/*!
    \qmlproperty int PdfNavigationStack::currentPage

    This property holds the current page that is being viewed.
    If there is no current page, it holds \c -1.
*/
int QQuickPdfNavigationStack::currentPage() const
{
    if (m_currentHistoryIndex < 0 || m_currentHistoryIndex >= m_pageHistory.count())
        return -1;
    return m_pageHistory.at(m_currentHistoryIndex)->page;
}

/*!
    \qmlproperty point PdfNavigationStack::currentLocation

    This property holds the current location on the page that is being viewed.
*/
QPointF QQuickPdfNavigationStack::currentLocation() const
{
    if (m_currentHistoryIndex < 0 || m_currentHistoryIndex >= m_pageHistory.count())
        return QPointF();
    return m_pageHistory.at(m_currentHistoryIndex)->location;
}

/*!
    \qmlproperty real PdfNavigationStack::currentZoom

    This property holds the magnification scale on the page that is being viewed.
*/
qreal QQuickPdfNavigationStack::currentZoom() const
{
    if (m_currentHistoryIndex < 0 || m_currentHistoryIndex >= m_pageHistory.count())
        return 1;
    return m_pageHistory.at(m_currentHistoryIndex)->zoom;
}

/*!
    \qmlmethod void PdfNavigationStack::push(int page, point location, qreal zoom)

    Adds the given destination, consisting of \a page, \a location and \a zoom,
    to the history of visited locations.  If \a emitJumped is \c false, the
    \l jumped() signal will not be emitted.

    If forwardAvailable is \c true, calling this function represents a branch
    in the timeline which causes the "future" to be lost, and therefore
    forwardAvailable will change to \c false.
*/
void QQuickPdfNavigationStack::push(int page, QPointF location, qreal zoom, bool emitJumped)
{
    if (page == currentPage() && location == currentLocation() && zoom == currentZoom())
        return;
    if (qFuzzyIsNull(zoom))
        zoom = currentZoom();
    bool backAvailableWas = backAvailable();
    bool forwardAvailableWas = forwardAvailable();
    if (!m_changing) {
        if (m_currentHistoryIndex >= 0 && forwardAvailableWas)
            m_pageHistory.remove(m_currentHistoryIndex + 1, m_pageHistory.count() - m_currentHistoryIndex - 1);
        m_pageHistory.append(QExplicitlySharedDataPointer<QPdfDestinationPrivate>(new QPdfDestinationPrivate(page, location, zoom)));
        m_currentHistoryIndex = m_pageHistory.count() - 1;
    }
    emit currentZoomChanged();
    emit currentPageChanged();
    emit currentLocationChanged();
    if (m_changing)
        return;
    if (!backAvailableWas)
        emit backAvailableChanged();
    if (forwardAvailableWas)
        emit forwardAvailableChanged();
    if (emitJumped)
        emit jumped(page, location, zoom);
    qCDebug(qLcNav) << "push: index" << m_currentHistoryIndex << "page" << page
                    << "@" << location << "zoom" << zoom << "-> history" <<
        [this]() {
            QStringList ret;
            for (auto d : m_pageHistory)
                ret << QString::number(d->page);
            return ret.join(',');
        }();
}

/*!
    \qmlmethod void PdfNavigationStack::update(int page, point location, qreal zoom)

    Modifies the current destination, consisting of \a page, \a location and \a zoom.

    This can be called periodically while the user is manually moving around
    the document, so that after back() is called, forward() will jump back to
    the most-recently-viewed destination rather than the destination that was
    last specified by push().

    The \c currentZoomChanged, \c currentPageChanged and \c currentLocationChanged
    signals will be emitted if the respective properties are actually changed.
    The \l jumped signal is not emitted, because this operation
    represents smooth movement rather than a navigational jump.
*/
void QQuickPdfNavigationStack::update(int page, QPointF location, qreal zoom)
{
    if (m_currentHistoryIndex < 0 || m_currentHistoryIndex >= m_pageHistory.count())
        return;
    int currentPageWas = currentPage();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    if (page == currentPageWas && location == currentLocationWas && zoom == currentZoomWas)
        return;
    m_pageHistory[m_currentHistoryIndex]->page = page;
    m_pageHistory[m_currentHistoryIndex]->location = location;
    m_pageHistory[m_currentHistoryIndex]->zoom = zoom;
    if (currentZoomWas != zoom)
        emit currentZoomChanged();
    if (currentPageWas != page)
        emit currentPageChanged();
    if (currentLocationWas != location)
        emit currentLocationChanged();
    qCDebug(qLcNav) << "update: index" << m_currentHistoryIndex << "page" << page
                    << "@" << location << "zoom" << zoom << "-> history" <<
        [this]() {
            QStringList ret;
            for (auto d : m_pageHistory)
                ret << QString::number(d->page);
            return ret.join(',');
        }();
}

bool QQuickPdfNavigationStack::backAvailable() const
{
    return m_currentHistoryIndex > 0;
}

bool QQuickPdfNavigationStack::forwardAvailable() const
{
    return m_currentHistoryIndex < m_pageHistory.count() - 1;
}

/*!
    \qmlsignal PdfNavigationStack::jumped(int page, point location, qreal zoom)

    This signal is emitted when forward(), back() or push() is called, but not
    when update() is called.
*/

QT_END_NAMESPACE
