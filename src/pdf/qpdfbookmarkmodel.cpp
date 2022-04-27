/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
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

#include "qpdfbookmarkmodel.h"

#include "qpdfdocument.h"
#include "qpdfdocument_p.h"

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdfview.h"

#include <QLoggingCategory>
#include <QMetaEnum>
#include <QPointer>
#include <QScopedPointer>
#include <private/qabstractitemmodel_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcBM, "qt.pdf.bookmarks")

class BookmarkNode
{
public:
    explicit BookmarkNode(BookmarkNode *parentNode = nullptr)
        : m_parentNode(parentNode)
    {
    }

    ~BookmarkNode()
    {
        clear();
    }

    void clear()
    {
        qDeleteAll(m_childNodes);
        m_childNodes.clear();
    }

    void appendChild(BookmarkNode *child)
    {
        m_childNodes.append(child);
    }

    BookmarkNode *child(int row) const
    {
        return m_childNodes.at(row);
    }

    int childCount() const
    {
        return m_childNodes.count();
    }

    int row() const
    {
        if (m_parentNode)
            return m_parentNode->m_childNodes.indexOf(const_cast<BookmarkNode*>(this));

        return 0;
    }

    BookmarkNode *parentNode() const
    {
        return m_parentNode;
    }

    QString title() const
    {
        return m_title;
    }

    void setTitle(const QString &title)
    {
        m_title = title;
    }

    int level() const
    {
        return m_level;
    }

    void setLevel(int level)
    {
        m_level = level;
    }

    int pageNumber() const
    {
        return m_pageNumber;
    }

    void setPageNumber(int pageNumber)
    {
        m_pageNumber = pageNumber;
    }

    QPointF location() const
    {
        return m_location;
    }

    void setLocation(qreal x, qreal y)
    {
        m_location = QPointF(x, y);
    }

    qreal zoom() const
    {
        return m_zoom;
    }

    void setZoom(qreal zoom)
    {
        m_zoom = zoom;
    }

private:
    QList<BookmarkNode*> m_childNodes;
    BookmarkNode *m_parentNode;

    QString m_title;
    int m_level = 0;
    int m_pageNumber = 0;
    QPointF m_location;
    qreal m_zoom = 0;
};


struct QPdfBookmarkModelPrivate
{
    QPdfBookmarkModelPrivate()
        : m_rootNode(new BookmarkNode(nullptr))
        , m_document(nullptr)
    {
    }

    void rebuild()
    {
        const bool documentAvailable = (m_document && m_document->status() == QPdfDocument::Status::Ready);

        if (documentAvailable) {
            q->beginResetModel();
            m_rootNode->clear();
            QPdfMutexLocker lock;
            appendChildNode(m_rootNode.data(), nullptr, 0, m_document->d->doc);
            lock.unlock();
            q->endResetModel();
        } else {
            if (m_rootNode->childCount() == 0) {
                return;
            } else {
                q->beginResetModel();
                m_rootNode->clear();
                q->endResetModel();
            }
        }
    }

    void appendChildNode(BookmarkNode *parentBookmarkNode, FPDF_BOOKMARK parentBookmark, int level, FPDF_DOCUMENT document)
    {
        FPDF_BOOKMARK bookmark = FPDFBookmark_GetFirstChild(document, parentBookmark);

        while (bookmark) {
            BookmarkNode *childBookmarkNode = nullptr;

            childBookmarkNode = new BookmarkNode(parentBookmarkNode);
            parentBookmarkNode->appendChild(childBookmarkNode);
            Q_ASSERT(childBookmarkNode);

            const int titleLength = int(FPDFBookmark_GetTitle(bookmark, nullptr, 0));

            QList<char16_t> titleBuffer(titleLength);
            FPDFBookmark_GetTitle(bookmark, titleBuffer.data(), quint32(titleBuffer.length()));

            const FPDF_DEST dest = FPDFBookmark_GetDest(document, bookmark);
            const int pageNumber = FPDFDest_GetDestPageIndex(document, dest);
            double pageHeight = 11.69 * 72; // A4 height
            {
                // get actual page height
                const QPdfMutexLocker lock;
                FPDF_PAGE pdfPage = FPDF_LoadPage(document, pageNumber);
                if (pdfPage)
                    pageHeight = FPDF_GetPageHeight(pdfPage);
                else
                    qCWarning(qLcBM) << "failed to load page" << pageNumber;
            }

            FPDF_BOOL hasX, hasY, hasZoom;
            FS_FLOAT x, y, zoom;
            bool ok = FPDFDest_GetLocationInPage(dest, &hasX, &hasY, &hasZoom, &x, &y, &zoom);
            if (ok) {
                if (hasX && hasY)
                    childBookmarkNode->setLocation(x, pageHeight - y);
                if (hasZoom)
                    childBookmarkNode->setZoom(zoom);
            } else {
                qCWarning(qLcBM) << "bookmark with invalid location and/or zoom" << x << y << zoom;
            }

            childBookmarkNode->setTitle(QString::fromUtf16(titleBuffer.data()));
            childBookmarkNode->setLevel(level);
            childBookmarkNode->setPageNumber(pageNumber);

            // recurse down
            appendChildNode(childBookmarkNode, bookmark, level + 1, document);

            bookmark = FPDFBookmark_GetNextSibling(document, bookmark);
        }
    }

    void _q_documentStatusChanged()
    {
        rebuild();
    }

    QPdfBookmarkModel *q = nullptr;

    QScopedPointer<BookmarkNode> m_rootNode;
    QPointer<QPdfDocument> m_document;
};


/*!
    \class QPdfBookmarkModel
    \since 5.10
    \inmodule QtPdf
    \inherits QAbstractItemModel

    \brief The QPdfBookmarkModel class holds a tree of of links (anchors)
    within a PDF document, such as the table of contents.

    This is used in the \l {Model/View Programming} paradigm to display a
    table of contents in the form of a tree or list.
*/

/*!
    \enum QPdfBookmarkModel::Role

    \value Title The name of the bookmark for display.
    \value Level The level of indentation.
    \value Page The page number of the destination (int).
    \value Location The position of the destination (QPointF).
    \value Zoom The suggested zoom level (qreal).
    \omitvalue _Count
*/

/*!
    Constructs a new bookmark model with parent object \a parent.
*/
QPdfBookmarkModel::QPdfBookmarkModel(QObject *parent)
    : QAbstractItemModel(parent), d(new QPdfBookmarkModelPrivate)
{
    d->q = this;
    m_roleNames = QAbstractItemModel::roleNames();
    QMetaEnum rolesMetaEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));
    for (int r = Qt::UserRole; r < int(Role::_Count); ++r)
        m_roleNames.insert(r, QByteArray(rolesMetaEnum.valueToKey(r)).toLower());
}

/*!
    Destroys the model.
*/
QPdfBookmarkModel::~QPdfBookmarkModel() = default;

QPdfDocument* QPdfBookmarkModel::document() const
{
    return d->m_document;
}

void QPdfBookmarkModel::setDocument(QPdfDocument *document)
{
    if (d->m_document == document)
        return;

    if (d->m_document)
        disconnect(d->m_document, SIGNAL(statusChanged(QPdfDocument::Status)), this, SLOT(_q_documentStatusChanged()));

    d->m_document = document;
    emit documentChanged(d->m_document);

    if (d->m_document)
        connect(d->m_document, SIGNAL(statusChanged(QPdfDocument::Status)), this, SLOT(_q_documentStatusChanged()));

    d->rebuild();
}

/*!
    \reimp
*/
int QPdfBookmarkModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

/*!
    \reimp
*/
QHash<int, QByteArray> QPdfBookmarkModel::roleNames() const
{
    return m_roleNames;
}

/*!
    \reimp
*/
QVariant QPdfBookmarkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const BookmarkNode *node = static_cast<BookmarkNode*>(index.internalPointer());
    switch (Role(role)) {
    case Role::Title:
        return node->title();
    case Role::Level:
        return node->level();
    case Role::Page:
        return node->pageNumber();
    case Role::Location:
        return node->location();
    case Role::Zoom:
        return node->zoom();
    case Role::_Count:
        break;
    }
    if (role == Qt::DisplayRole)
        return node->title();
    return QVariant();
}

/*!
    \reimp
*/
QModelIndex QPdfBookmarkModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    BookmarkNode *parentNode;

    if (!parent.isValid())
        parentNode = d->m_rootNode.data();
    else
        parentNode = static_cast<BookmarkNode*>(parent.internalPointer());

    BookmarkNode *childNode = parentNode->child(row);
    if (childNode)
        return createIndex(row, column, childNode);
    else
        return QModelIndex();
}

/*!
    \reimp
*/
QModelIndex QPdfBookmarkModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    const BookmarkNode *childNode = static_cast<BookmarkNode*>(index.internalPointer());
    BookmarkNode *parentNode = childNode->parentNode();

    if (parentNode == d->m_rootNode.data())
        return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

/*!
    \reimp
*/
int QPdfBookmarkModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    BookmarkNode *parentNode = nullptr;

    if (!parent.isValid())
        parentNode = d->m_rootNode.data();
    else
        parentNode = static_cast<BookmarkNode*>(parent.internalPointer());

    return parentNode->childCount();
}

QT_END_NAMESPACE

#include "moc_qpdfbookmarkmodel.cpp"
