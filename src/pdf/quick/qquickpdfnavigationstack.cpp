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
    \brief History of the pages visited within a PDF Document.
    \since 5.15

    PdfNavigationStack remembers which pages the user has visited in a PDF
    document, and provides the ability to traverse backward and forward.
*/

QQuickPdfNavigationStack::QQuickPdfNavigationStack(QObject *parent)
    : QObject(parent)
{
}

/*!
    \qmlmethod void PdfNavigationStack::forward()

    Goes back to the page that was being viewed before back() was called, and
    then emits the \l currentPageJumped() signal.

    If \l currentPage was set by assignment or binding since the last time
    \l back() was called, the forward() function does nothing, because there is
    a branch in the timeline which causes the "future" to be lost.
*/
void QQuickPdfNavigationStack::forward()
{
    if (m_nextHistoryIndex >= m_pageHistory.count())
        return;
    bool backAvailableWas = backAvailable();
    bool forwardAvailableWas = forwardAvailable();
    m_changing = true;
    setCurrentPage(m_pageHistory.at(++m_nextHistoryIndex));
    m_changing = false;
    emit currentPageJumped(m_currentPage);
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged();
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged();
}

/*!
    \qmlmethod void PdfNavigationStack::back()

    Pops the stack and causes the \l currentPage property to change to the
    most-recently-viewed page, and then emits the \l currentPageJumped()
    signal.
*/
void QQuickPdfNavigationStack::back()
{
    if (m_nextHistoryIndex <= 0)
        return;
    bool backAvailableWas = backAvailable();
    bool forwardAvailableWas = forwardAvailable();
    m_changing = true;
    // TODO don't do that when going back after going forward
    m_pageHistory.append(m_currentPage);
    setCurrentPage(m_pageHistory.at(--m_nextHistoryIndex));
    m_changing = false;
    emit currentPageJumped(m_currentPage);
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged();
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged();
}

/*!
    \qmlproperty int PdfNavigationStack::currentPage

    This property holds the current page that is being viewed.

    It should be set when the viewer's current page changes. Every time this
    property is set, it pushes the current page number onto the stack, such
    that the history of pages that have been viewed will grow.
*/
void QQuickPdfNavigationStack::setCurrentPage(int currentPage)
{
    if (m_currentPage == currentPage)
        return;
    bool backAvailableWas = backAvailable();
    bool forwardAvailableWas = forwardAvailable();
    if (!m_changing) {
        if (m_nextHistoryIndex >= 0 && m_nextHistoryIndex < m_pageHistory.count())
            m_pageHistory.remove(m_nextHistoryIndex, m_pageHistory.count() - m_nextHistoryIndex);
        m_pageHistory.append(m_currentPage);
        m_nextHistoryIndex = m_pageHistory.count();
    }
    m_currentPage = currentPage;
    emit currentPageChanged();
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged();
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged();
    qCDebug(qLcNav) << "current" << m_currentPage << "history" << m_pageHistory;
}

bool QQuickPdfNavigationStack::backAvailable() const
{
    return m_nextHistoryIndex > 0;
}

bool QQuickPdfNavigationStack::forwardAvailable() const
{
    return m_nextHistoryIndex < m_pageHistory.count();
}

/*!
    \qmlsignal PdfNavigationStack::currentPageJumped(int page)

    This signal is emitted when either forward() or back() is called, to
    distinguish navigational jumps from cases when the \l currentPage property
    is set by means of a binding or assignment. Contrast with the
    \c currentPageChanged signal, which is emitted in all cases, and does not
    include the \c page argument.
*/

QT_END_NAMESPACE
