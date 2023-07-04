// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfpageselector.h"
#include "qpdfpageselector_p.h"

#include <QPdfDocument>

QT_BEGIN_NAMESPACE

QPdfPageSelectorPrivate::QPdfPageSelectorPrivate(QPdfPageSelector *q)
    : q_ptr(q)
{
}

void QPdfPageSelectorPrivate::documentStatusChanged()
{
    Q_Q(QPdfPageSelector);
    if (!document.isNull() && document->status() == QPdfDocument::Status::Ready) {
        q->setMaximum(document->pageCount());
        q->setValue(0);
    }
}

/*!
    \class QPdfPageSelector
    \inmodule QtPdf
    \brief A QSpinBox for selecting a PDF page.

    QPdfPageSelector is a QSpinBox specialized for selecting a page label
    from a QPdfDocument.

    \sa QPdfDocument::pageLabel()
*/

/*!
    Constructs a PDF page selector with parent widget \a parent.
*/
QPdfPageSelector::QPdfPageSelector(QWidget *parent)
    : QSpinBox(parent)
    , d_ptr(new QPdfPageSelectorPrivate(this))
{
}

/*!
    Destroys the page selector.
*/
QPdfPageSelector::~QPdfPageSelector()
{
}

/*!
    \property QPdfPageSelector::document

    This property holds the document to be viewed.
*/
void QPdfPageSelector::setDocument(QPdfDocument *document)
{
    Q_D(QPdfPageSelector);

    if (d->document == document)
        return;

    if (d->document)
        disconnect(d->documentStatusChangedConnection);

    d->document = document;
    emit documentChanged(d->document);

    if (d->document)
        d->documentStatusChangedConnection =
                connect(d->document.data(), &QPdfDocument::statusChanged, this,
                        [d](){ d->documentStatusChanged(); });

    d->documentStatusChanged();
}

QPdfDocument *QPdfPageSelector::document() const
{
    Q_D(const QPdfPageSelector);

    return d->document;
}

int QPdfPageSelector::valueFromText(const QString &text) const
{
    Q_D(const QPdfPageSelector);
    if (d->document.isNull())
        return 0;

    return d->document->pageIndexForLabel(text.trimmed());
}

QString QPdfPageSelector::textFromValue(int value) const
{
    Q_D(const QPdfPageSelector);
    if (d->document.isNull())
        return {};

    return d->document->pageLabel(value);
}

QValidator::State QPdfPageSelector::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);
    return valueFromText(text) >= 0 ? QValidator::Acceptable : QValidator::Intermediate;
}

QT_END_NAMESPACE

#include "moc_qpdfpageselector.cpp"
