/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
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

#include "qpdfpagenavigation.h"

#include "qpdfdocument.h"

#include <private/qobject_p.h>

#include <QPointer>

QT_BEGIN_NAMESPACE

class QPdfPageNavigationPrivate : public QObjectPrivate
{
public:
    QPdfPageNavigationPrivate()
        : QObjectPrivate()
    {
    }

    void update()
    {
        Q_Q(QPdfPageNavigation);

        const bool documentAvailable = m_document && m_document->status() == QPdfDocument::Ready;

        if (documentAvailable) {
            const int newPageCount = m_document->pageCount();
            if (m_pageCount != newPageCount) {
                m_pageCount = newPageCount;
                emit q->pageCountChanged(m_pageCount);
            }
        } else {
            if (m_pageCount != 0) {
                m_pageCount = 0;
                emit q->pageCountChanged(m_pageCount);
            }
        }

        if (m_currentPage != 0) {
            m_currentPage = 0;
            emit q->currentPageChanged(m_currentPage);
        }

        updatePrevNext();
    }

    void updatePrevNext()
    {
        Q_Q(QPdfPageNavigation);

        const bool hasPreviousPage = m_currentPage > 0;
        const bool hasNextPage = m_currentPage < (m_pageCount - 1);

        if (m_canGoToPreviousPage != hasPreviousPage) {
            m_canGoToPreviousPage = hasPreviousPage;
            emit q->canGoToPreviousPageChanged(m_canGoToPreviousPage);
        }

        if (m_canGoToNextPage != hasNextPage) {
            m_canGoToNextPage = hasNextPage;
            emit q->canGoToNextPageChanged(m_canGoToNextPage);
        }
    }

    void documentStatusChanged()
    {
        update();
    }

    Q_DECLARE_PUBLIC(QPdfPageNavigation)

    QPointer<QPdfDocument> m_document = nullptr;
    int m_currentPage = 0;
    int m_pageCount = 0;
    bool m_canGoToPreviousPage = false;
    bool m_canGoToNextPage = false;

    QMetaObject::Connection m_documentStatusChangedConnection;
};

/*!
    \class QPdfPageNavigation
    \since 5.10
    \inmodule QtPdf

    \brief The QPdfPageNavigation class handles the navigation through a PDF document.

    \sa QPdfDocument
*/


/*!
    Constructs a page navigation object with parent object \a parent.
*/
QPdfPageNavigation::QPdfPageNavigation(QObject *parent)
    : QObject(*new QPdfPageNavigationPrivate, parent)
{
}

/*!
    Destroys the page navigation object.
*/
QPdfPageNavigation::~QPdfPageNavigation()
{
}

/*!
    \property QPdfPageNavigation::document
    \brief The document instance on which this object navigates.

    By default, this property is \c nullptr.

    \sa document(), setDocument(), QPdfDocument
*/

/*!
    Returns the document on which this object navigates, or a \c nullptr
    if none has set before.

    \sa QPdfDocument
*/
QPdfDocument* QPdfPageNavigation::document() const
{
    Q_D(const QPdfPageNavigation);

    return d->m_document;
}

/*!
    Sets the \a document this object navigates on.

    After a new document has been set, the currentPage will be \c 0.

    \sa QPdfDocument
*/
void QPdfPageNavigation::setDocument(QPdfDocument *document)
{
    Q_D(QPdfPageNavigation);

    if (d->m_document == document)
        return;

    if (d->m_document)
        disconnect(d->m_documentStatusChangedConnection);

    d->m_document = document;
    emit documentChanged(d->m_document);

    if (d->m_document)
        d->m_documentStatusChangedConnection = connect(d->m_document.data(), &QPdfDocument::statusChanged, this, [d](){ d->documentStatusChanged(); });

    d->update();
}

/*!
    \property QPdfPageNavigation::currentPage
    \brief The current page number in the document.

    \sa currentPage(), setCurrentPage()
*/

/*!
    Returns the current page number or \c 0 if there is no document set.

    After a document has been loaded, the currentPage will always be \c 0.
*/
int QPdfPageNavigation::currentPage() const
{
    Q_D(const QPdfPageNavigation);

    return d->m_currentPage;
}

/*!
    \fn void QPdfPageNavigation::setCurrentPage(int page)

    Sets the current \a page number.
*/
void QPdfPageNavigation::setCurrentPage(int newPage)
{
    Q_D(QPdfPageNavigation);

    if (newPage < 0 || newPage >= d->m_pageCount)
        return;

    if (d->m_currentPage == newPage)
        return;

    d->m_currentPage = newPage;
    emit currentPageChanged(d->m_currentPage);

    d->updatePrevNext();
}

/*!
    \property QPdfPageNavigation::pageCount
    \brief The number of pages in the document.

    \sa pageCount()
*/

/*!
    Returns the number of pages in the document or \c 0 if there
    is no document set.
*/
int QPdfPageNavigation::pageCount() const
{
    Q_D(const QPdfPageNavigation);

    return d->m_pageCount;
}

/*!
    \property QPdfPageNavigation::canGoToPreviousPage
    \brief Indicates whether there is a page before the current page.

    \sa canGoToPreviousPage(), goToPreviousPage()
*/

/*!
    Returns whether there is a page before the current one.
*/
bool QPdfPageNavigation::canGoToPreviousPage() const
{
    Q_D(const QPdfPageNavigation);

    return d->m_canGoToPreviousPage;
}

/*!
    \property QPdfPageNavigation::canGoToNextPage
    \brief Indicates whether there is a page after the current page.

    \sa canGoToNextPage(), goToNextPage()
*/

/*!
    Returns whether there is a page after the current one.
*/
bool QPdfPageNavigation::canGoToNextPage() const
{
    Q_D(const QPdfPageNavigation);

    return d->m_canGoToNextPage;
}

/*!
    Changes the current page to the previous page.

    If there is no previous page in the document, nothing happens.

    \sa canGoToPreviousPage
*/
void QPdfPageNavigation::goToPreviousPage()
{
    Q_D(QPdfPageNavigation);

    if (d->m_currentPage > 0)
        setCurrentPage(d->m_currentPage - 1);
}

/*!
    Changes the current page to the next page.

    If there is no next page in the document, nothing happens.

    \sa canGoToNextPage
*/
void QPdfPageNavigation::goToNextPage()
{
    Q_D(QPdfPageNavigation);

    if (d->m_currentPage < d->m_pageCount - 1)
        setCurrentPage(d->m_currentPage + 1);
}

QT_END_NAMESPACE

#include "moc_qpdfpagenavigation.cpp"
