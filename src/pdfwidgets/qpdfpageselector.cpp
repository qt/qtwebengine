// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfpageselector.h"
#include "qpdfpageselector_p.h"

#include <QPdfDocument>

#include <QtWidgets/qboxlayout.h>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
    \class QPdfPageSelector
    \inmodule QtPdf
    \since 6.6
    \brief A widget for selecting a PDF page.

    QPdfPageSelector is a widget for selecting a page label from a
    QPdfDocument.

    \sa QPdfDocument::pageLabel()
*/

/*!
    Constructs a PDF page selector with parent widget \a parent.
*/
QPdfPageSelector::QPdfPageSelector(QWidget *parent)
    : QWidget(parent), d_ptr(new QPdfPageSelectorPrivate)
{
    Q_D(QPdfPageSelector);
    d->spinBox = new QPdfPageSelectorSpinBox(this);
    d->spinBox->setObjectName(u"_q_spinBox"_s);
    auto vlay = new QVBoxLayout(this);
    vlay->setContentsMargins({});
    vlay->addWidget(d->spinBox);

    connect(d->spinBox, &QPdfPageSelectorSpinBox::_q_documentChanged,
            this, &QPdfPageSelector::documentChanged);
    connect(d->spinBox, &QSpinBox::valueChanged, this, &QPdfPageSelector::currentPageChanged);
    connect(d->spinBox, &QSpinBox::textChanged, this, &QPdfPageSelector::currentPageLabelChanged);
}

/*!
    Destroys the page selector.
*/
QPdfPageSelector::~QPdfPageSelector()
    = default;

/*!
    \property QPdfPageSelector::document

    This property holds the document to be viewed.
*/

void QPdfPageSelector::setDocument(QPdfDocument *doc)
{
    Q_D(QPdfPageSelector);
    d->spinBox->setDocument(doc);
}

QPdfDocument *QPdfPageSelector::document() const
{
    Q_D(const QPdfPageSelector);
    return d->spinBox->document();
}

/*!
    \property QPdfPageSelector::currentPage

    This property holds the index (\c{0}-based) of the current page in the
    document.
*/

int QPdfPageSelector::currentPage() const
{
    Q_D(const QPdfPageSelector);
    return d->spinBox->value();
}

void QPdfPageSelector::setCurrentPage(int index)
{
    Q_D(QPdfPageSelector);
    d->spinBox->setValue(index);
}

/*!
    \property QPdfPageSelector::currentPageIndex

    This property holds the page label corresponding to the current page index
    in the document.

    This is the text presented to the user.

    \sa QPdfDocument::pageLabel()
*/

QString QPdfPageSelector::currentPageLabel() const
{
    Q_D(const QPdfPageSelector);
    return d->spinBox->text();
}

//
// QPdfPageSelectorSpinBox:
//

void QPdfPageSelectorSpinBox::documentStatusChanged()
{
    if (m_document && m_document->status() == QPdfDocument::Status::Ready) {
        setMaximum(m_document->pageCount());
        setValue(0);
    }
}

void QPdfPageSelectorSpinBox::setDocument(QPdfDocument *document)
{
    if (m_document == document)
        return;

    if (m_document)
        disconnect(m_documentStatusChangedConnection);

    m_document = document;
    emit _q_documentChanged(document);

    if (m_document) {
        m_documentStatusChangedConnection =
                connect(m_document.get(), &QPdfDocument::statusChanged,
                        this, &QPdfPageSelectorSpinBox::documentStatusChanged);
    }

    documentStatusChanged();
}

QPdfPageSelectorSpinBox::QPdfPageSelectorSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
}

QPdfPageSelectorSpinBox::~QPdfPageSelectorSpinBox()
    = default;

int QPdfPageSelectorSpinBox::valueFromText(const QString &text) const
{
    if (!m_document)
        return 0;

    return m_document->pageIndexForLabel(text.trimmed());
}

QString QPdfPageSelectorSpinBox::textFromValue(int value) const
{
    if (!m_document)
        return {};

    return m_document->pageLabel(value);
}

QValidator::State QPdfPageSelectorSpinBox::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);
    return valueFromText(text) >= 0 ? QValidator::Acceptable : QValidator::Intermediate;
}

QT_END_NAMESPACE

#include "moc_qpdfpageselector_p.cpp"
#include "moc_qpdfpageselector.cpp"
