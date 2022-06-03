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

#include "qpdflink_p.h"
#include "qpdflinkmodel_p.h"
#include "qpdflinkmodel_p_p.h"
#include "qpdfdocument_p.h"

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdf_text.h"

#include <QLoggingCategory>
#include <QMetaEnum>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcLink, "qt.pdf.links")

QPdfLinkModel::QPdfLinkModel(QObject *parent)
    : QAbstractListModel(*(new QPdfLinkModelPrivate()), parent)
{
    QMetaEnum rolesMetaEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));
    for (int r = Qt::UserRole; r < int(Role::_Count); ++r)
        m_roleNames.insert(r, QByteArray(rolesMetaEnum.valueToKey(r)).toLower());
}

QPdfLinkModel::~QPdfLinkModel() {}

QHash<int, QByteArray> QPdfLinkModel::roleNames() const
{
    return m_roleNames;
}

int QPdfLinkModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QPdfLinkModel);
    Q_UNUSED(parent);
    return d->links.count();
}

QVariant QPdfLinkModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QPdfLinkModel);
    const auto &link = d->links.at(index.row());
    switch (Role(role)) {
    case Role::Link:
        return QVariant::fromValue(link);
    case Role::Rectangle:
        return link.rectangles().empty() ? QVariant() : link.rectangles().constFirst();
    case Role::Url:
        return link.url();
    case Role::Page:
        return link.page();
    case Role::Location:
        return link.location();
    case Role::Zoom:
        return link.zoom();
    case Role::_Count:
        break;
    }
    if (role == Qt::DisplayRole)
        return link.toString();
    return QVariant();
}

QPdfDocument *QPdfLinkModel::document() const
{
    Q_D(const QPdfLinkModel);
    return d->document;
}

void QPdfLinkModel::setDocument(QPdfDocument *document)
{
    Q_D(QPdfLinkModel);
    if (d->document == document)
        return;
    if (d->document)
        disconnect(d->document, &QPdfDocument::statusChanged, this, &QPdfLinkModel::onStatusChanged);
    connect(document, &QPdfDocument::statusChanged, this, &QPdfLinkModel::onStatusChanged);
    d->document = document;
    emit documentChanged();
    if (page())
        setPage(0);
    else
        d->update();
}

int QPdfLinkModel::page() const
{
    Q_D(const QPdfLinkModel);
    return d->page;
}

void QPdfLinkModel::setPage(int page)
{
    Q_D(QPdfLinkModel);
    if (d->page == page)
        return;

    d->page = page;
    emit pageChanged(page);
    d->update();
}

QPdfLinkModelPrivate::QPdfLinkModelPrivate() : QAbstractItemModelPrivate()
{
}

void QPdfLinkModelPrivate::update()
{
    Q_Q(QPdfLinkModel);
    if (!document || !document->d->doc)
        return;
    auto doc = document->d->doc;
    const QPdfMutexLocker lock;
    FPDF_PAGE pdfPage = FPDF_LoadPage(doc, page);
    if (!pdfPage) {
        qCWarning(qLcLink) << "failed to load page" << page;
        return;
    }
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    q->beginResetModel();
    links.clear();

    // Iterate the ordinary links
    int linkStart = 0;
    bool hasNext = true;
    while (hasNext) {
        FPDF_LINK linkAnnot;
        hasNext = FPDFLink_Enumerate(pdfPage, &linkStart, &linkAnnot);
        if (!hasNext)
            break;
        FS_RECTF rect;
        bool ok = FPDFLink_GetAnnotRect(linkAnnot, &rect);
        if (!ok) {
            qCWarning(qLcLink) << "skipping link with invalid bounding box";
            continue; // while enumerating links
        }
        QPdfLink linkData;
        linkData.d->rects << QRectF(rect.left, pageHeight - rect.top,
                               rect.right - rect.left, rect.top - rect.bottom);
        FPDF_DEST dest = FPDFLink_GetDest(doc, linkAnnot);
        FPDF_ACTION action = FPDFLink_GetAction(linkAnnot);
        switch (FPDFAction_GetType(action)) {
        case PDFACTION_UNSUPPORTED: // this happens with valid links in some PDFs
        case PDFACTION_GOTO: {
            linkData.d->page = FPDFDest_GetDestPageIndex(doc, dest);
            if (linkData.d->page < 0) {
                qCWarning(qLcLink) << "skipping link with invalid page number";
                continue; // while enumerating links
            }
            FPDF_BOOL hasX, hasY, hasZoom;
            FS_FLOAT x, y, zoom;
            ok = FPDFDest_GetLocationInPage(dest, &hasX, &hasY, &hasZoom, &x, &y, &zoom);
            if (!ok) {
                qCWarning(qLcLink) << "link with invalid location and/or zoom @" << linkData.d->rects;
                break; // at least we got a page number, so the link will jump there
            }
            if (hasX && hasY)
                linkData.d->location = QPointF(x, pageHeight - y);
            if (hasZoom)
                linkData.d->zoom = zoom;
            break;
        }
        case PDFACTION_URI: {
            unsigned long len = FPDFAction_GetURIPath(doc, action, nullptr, 0);
            if (len < 1) {
                qCWarning(qLcLink) << "skipping link with empty URI @" << linkData.d->rects;
                continue; // while enumerating links
            } else {
                QByteArray buf(len, 0);
                unsigned long got = FPDFAction_GetURIPath(doc, action, buf.data(), len);
                Q_ASSERT(got == len);
                linkData.d->url = QString::fromLatin1(buf.data(), got - 1);
            }
            break;
        }
        case PDFACTION_LAUNCH:
        case PDFACTION_REMOTEGOTO: {
            unsigned long len = FPDFAction_GetFilePath(action, nullptr, 0);
            if (len < 1) {
                qCWarning(qLcLink) << "skipping link with empty file path @" << linkData.d->rects;
                continue; // while enumerating links
            } else {
                QByteArray buf(len, 0);
                unsigned long got = FPDFAction_GetFilePath(action, buf.data(), len);
                Q_ASSERT(got == len);
                linkData.d->url = QUrl::fromLocalFile(QString::fromLatin1(buf.data(), got - 1)).toString();

                // Unfortunately, according to comments in fpdf_doc.h, if it's PDFACTION_REMOTEGOTO,
                // we can't get the page and location without first opening the linked document
                // and then calling FPDFAction_GetDest() again.
            }
            break;
        }
        }
        links << linkData;
    }

    // Iterate the web links
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    if (textPage) {
        FPDF_PAGELINK webLinks = FPDFLink_LoadWebLinks(textPage);
        if (webLinks) {
            int count = FPDFLink_CountWebLinks(webLinks);
            for (int i = 0; i < count; ++i) {
                QPdfLink linkData;
                int len = FPDFLink_GetURL(webLinks, i, nullptr, 0);
                if (len < 1) {
                    qCWarning(qLcLink) << "skipping link" << i << "with empty URL";
                } else {
                    QList<unsigned short> buf(len);
                    int got = FPDFLink_GetURL(webLinks, i, buf.data(), len);
                    Q_ASSERT(got == len);
                    linkData.d->url = QString::fromUtf16(
                            reinterpret_cast<const char16_t *>(buf.data()), got - 1);
                }
                len = FPDFLink_CountRects(webLinks, i);
                for (int r = 0; r < len; ++r) {
                    double left, top, right, bottom;
                    bool success = FPDFLink_GetRect(webLinks, i, r, &left, &top, &right, &bottom);
                    if (success) {
                        linkData.d->rects << QRectF(left, pageHeight - top, right - left, top - bottom);
                        links << linkData;
                    }
                }
            }
            FPDFLink_CloseWebLinks(webLinks);
        }
        FPDFText_ClosePage(textPage);
    }

    // All done
    FPDF_ClosePage(pdfPage);
    if (Q_UNLIKELY(qLcLink().isDebugEnabled())) {
        for (const auto &l : links)
            qCDebug(qLcLink) << l;
    }
    q->endResetModel();
}

void QPdfLinkModel::onStatusChanged(QPdfDocument::Status status)
{
    Q_D(QPdfLinkModel);
    qCDebug(qLcLink) << "sees document statusChanged" << status;
    if (status == QPdfDocument::Status::Ready)
        d->update();
}

QT_END_NAMESPACE

#include "moc_qpdflinkmodel_p.cpp"
