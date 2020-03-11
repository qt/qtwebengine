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

#include "qpdfdestination.h"
#include "qpdfdocument_p.h"
#include "qpdfsearchmodel.h"
#include "qpdfsearchmodel_p.h"
#include "qpdfsearchresult_p.h"

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdf_text.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/QMetaEnum>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcS, "qt.pdf.search")

static const int UpdateTimerInterval = 100;
static const int ContextChars = 64;
static const double CharacterHitTolerance = 6.0;

QPdfSearchModel::QPdfSearchModel(QObject *parent)
    : QAbstractListModel(*(new QPdfSearchModelPrivate()), parent)
{
    QMetaEnum rolesMetaEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));
    for (int r = Qt::UserRole; r < int(Role::_Count); ++r) {
        QByteArray roleName = QByteArray(rolesMetaEnum.valueToKey(r));
        if (roleName.isEmpty())
            continue;
        roleName[0] = QChar::toLower(roleName[0]);
        m_roleNames.insert(r, roleName);
    }
}

QPdfSearchModel::~QPdfSearchModel() {}

QHash<int, QByteArray> QPdfSearchModel::roleNames() const
{
    return m_roleNames;
}

int QPdfSearchModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QPdfSearchModel);
    Q_UNUSED(parent)
    return d->rowCountSoFar;
}

QVariant QPdfSearchModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QPdfSearchModel);
    const auto pi = const_cast<QPdfSearchModelPrivate*>(d)->pageAndIndexForResult(index.row());
    if (pi.page < 0)
        return QVariant();
    switch (Role(role)) {
    case Role::Page:
        return pi.page;
    case Role::IndexOnPage:
        return pi.index;
    case Role::Location:
        return d->searchResults[pi.page][pi.index].location();
    case Role::ContextBefore:
        return d->searchResults[pi.page][pi.index].contextBefore();
    case Role::ContextAfter:
        return d->searchResults[pi.page][pi.index].contextAfter();
    case Role::_Count:
        break;
    }
    if (role == Qt::DisplayRole) {
        const QString ret = d->searchResults[pi.page][pi.index].contextBefore() +
                QLatin1String("<b>") + d->searchString + QLatin1String("</b>") +
                d->searchResults[pi.page][pi.index].contextAfter();
        return ret;
    }
    return QVariant();
}

void QPdfSearchModel::updatePage(int page)
{
    Q_D(QPdfSearchModel);
    d->doSearch(page);
}

QString QPdfSearchModel::searchString() const
{
    Q_D(const QPdfSearchModel);
    return d->searchString;
}

void QPdfSearchModel::setSearchString(QString searchString)
{
    Q_D(QPdfSearchModel);
    if (d->searchString == searchString)
        return;

    d->searchString = searchString;
    beginResetModel();
    d->clearResults();
    emit searchStringChanged();
    endResetModel();
}

QVector<QPdfSearchResult> QPdfSearchModel::resultsOnPage(int page) const
{
    Q_D(const QPdfSearchModel);
    const_cast<QPdfSearchModelPrivate *>(d)->doSearch(page);
    if (d->searchResults.count() <= page)
        return {};
    return d->searchResults[page];
}

QPdfSearchResult QPdfSearchModel::resultAtIndex(int index) const
{
    Q_D(const QPdfSearchModel);
    const auto pi = const_cast<QPdfSearchModelPrivate*>(d)->pageAndIndexForResult(index);
    if (pi.page < 0)
        return QPdfSearchResult();
    return d->searchResults[pi.page][pi.index];
}

QPdfDocument *QPdfSearchModel::document() const
{
    Q_D(const QPdfSearchModel);
    return d->document;
}

void QPdfSearchModel::setDocument(QPdfDocument *document)
{
    Q_D(QPdfSearchModel);
    if (d->document == document)
        return;

    d->document = document;
    d->clearResults();
    emit documentChanged();
}

void QPdfSearchModel::timerEvent(QTimerEvent *event)
{
    Q_D(QPdfSearchModel);
    if (event->timerId() != d->updateTimerId)
        return;
    if (!d->document || d->nextPageToUpdate >= d->document->pageCount()) {
        if (d->document)
            qCDebug(qLcS, "done updating search results on %d pages", d->searchResults.count());
        killTimer(d->updateTimerId);
        d->updateTimerId = -1;
    }
    d->doSearch(d->nextPageToUpdate++);
}

QPdfSearchModelPrivate::QPdfSearchModelPrivate() : QAbstractItemModelPrivate()
{
}

void QPdfSearchModelPrivate::clearResults()
{
    Q_Q(QPdfSearchModel);
    rowCountSoFar = 0;
    searchResults.clear();
    pagesSearched.clear();
    if (document) {
        searchResults.resize(document->pageCount());
        pagesSearched.resize(document->pageCount());
    } else {
        searchResults.resize(0);
        pagesSearched.resize(0);
    }
    nextPageToUpdate = 0;
    updateTimerId = q->startTimer(UpdateTimerInterval);
}

bool QPdfSearchModelPrivate::doSearch(int page)
{
    if (page < 0 || page >= pagesSearched.count() || searchString.isEmpty())
        return false;
    if (pagesSearched[page])
        return true;
    Q_Q(QPdfSearchModel);

    const QPdfMutexLocker lock;
    QElapsedTimer timer;
    timer.start();
    FPDF_PAGE pdfPage = FPDF_LoadPage(document->d->doc, page);
    if (!pdfPage) {
        qWarning() << "failed to load page" << page;
        return false;
    }
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    if (!textPage) {
        qWarning() << "failed to load text of page" << page;
        FPDF_ClosePage(pdfPage);
        return false;
    }
    FPDF_SCHHANDLE sh = FPDFText_FindStart(textPage, searchString.utf16(), 0, 0);
    QVector<QPdfSearchResult> newSearchResults;
    while (FPDFText_FindNext(sh)) {
        int idx = FPDFText_GetSchResultIndex(sh);
        int count = FPDFText_GetSchCount(sh);
        int rectCount = FPDFText_CountRects(textPage, idx, count);
        QVector<QRectF> rects;
        int startIndex = -1;
        int endIndex = -1;
        for (int r = 0; r < rectCount; ++r) {
            double left, top, right, bottom;
            FPDFText_GetRect(textPage, r, &left, &top, &right, &bottom);
            rects << QRectF(left, pageHeight - top, right - left, top - bottom);
            if (r == 0) {
                startIndex = FPDFText_GetCharIndexAtPos(textPage, left, top,
                        CharacterHitTolerance, CharacterHitTolerance);
            }
            if (r == rectCount - 1) {
                endIndex = FPDFText_GetCharIndexAtPos(textPage, right, top,
                        CharacterHitTolerance, CharacterHitTolerance);
            }
            qCDebug(qLcS) << rects.last() << "char idx" << startIndex << "->" << endIndex;
        }
        QString contextBefore, contextAfter;
        if (startIndex >= 0 || endIndex >= 0) {
            startIndex = qMax(0, startIndex - ContextChars);
            endIndex += ContextChars;
            int count = endIndex - startIndex + 1;
            if (count > 0) {
                QVector<ushort> buf(count + 1);
                int len = FPDFText_GetText(textPage, startIndex, count, buf.data());
                Q_ASSERT(len - 1 <= count); // len is number of characters written, including the terminator
                QString context = QString::fromUtf16(buf.constData(), len - 1);
                context = context.replace(QLatin1Char('\n'), QStringLiteral("\u23CE"));
                context = context.remove(QLatin1Char('\r'));
                // try to find the search string near the middle of the context if possible
                int si = context.indexOf(searchString, ContextChars - 5, Qt::CaseInsensitive);
                if (si < 0)
                    si = context.indexOf(searchString, Qt::CaseInsensitive);
                if (si < 0)
                    qWarning() << "search string" << searchString << "not found in context" << context;
                contextBefore = context.mid(0, si);
                contextAfter = context.mid(si + searchString.length());
            }
        }
        if (!rects.isEmpty())
            newSearchResults << QPdfSearchResult(page, rects, contextBefore, contextAfter);
    }
    FPDFText_FindClose(sh);
    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(pdfPage);
    qCDebug(qLcS) << searchString << "took" << timer.elapsed() << "ms to find"
                  << newSearchResults.count() << "results on page" << page;

    pagesSearched[page] = true;
    searchResults[page] = newSearchResults;
    if (newSearchResults.count() > 0) {
        int rowsBefore = rowsBeforePage(page);
        qCDebug(qLcS) << "from row" << rowsBefore << "rowCount" << rowCountSoFar << "increasing by" << newSearchResults.count();
        rowCountSoFar += newSearchResults.count();
        q->beginInsertRows(QModelIndex(), rowsBefore, rowsBefore + newSearchResults.count() - 1);
        q->endInsertRows();
    }
    return true;
}

QPdfSearchModelPrivate::PageAndIndex QPdfSearchModelPrivate::pageAndIndexForResult(int resultIndex)
{
    const int pageCount = document->pageCount();
    int totalSoFar = 0;
    int previousTotalSoFar = 0;
    for (int page = 0; page < pageCount; ++page) {
        if (!pagesSearched[page])
            doSearch(page);
        totalSoFar += searchResults[page].count();
        if (totalSoFar > resultIndex)
            return {page, resultIndex - previousTotalSoFar};
        previousTotalSoFar = totalSoFar;
    }
    return {-1, -1};
}

int QPdfSearchModelPrivate::rowsBeforePage(int page)
{
    int ret = 0;
    for (int i = 0; i < page; ++i)
        ret += searchResults[i].count();
    return ret;
}

QT_END_NAMESPACE

#include "moc_qpdfsearchmodel.cpp"
