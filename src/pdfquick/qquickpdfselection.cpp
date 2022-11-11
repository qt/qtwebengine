// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdfselection_p.h"
#include "qquickpdfdocument_p.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQuickItem>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtPdf/private/qpdfdocument_p.h>

Q_LOGGING_CATEGORY(qLcIm, "qt.pdf.im")

QT_BEGIN_NAMESPACE

static const QRegularExpression WordDelimiter(QStringLiteral("\\s"));

/*!
    \qmltype PdfSelection
//!    \instantiates QQuickPdfSelection
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \inherits Item
    \brief A representation of a text selection within a PDF Document.
    \since 5.15

    PdfSelection provides the text string and its geometry within a bounding box
    from one point to another.

    To modify the selection using the mouse, bind \l from and \l to
    to the suitable properties of an input handler so that they will be set to
    the positions where the drag gesture begins and ends, respectively; and
    bind the \l hold property so that it will be set to \c true during the drag
    gesture and \c false when the gesture ends.

    PdfSelection also directly handles Input Method queries so that text
    selection handles can be used on platforms such as iOS. For this purpose,
    it must have keyboard focus.
*/

QQuickPdfSelection::QQuickPdfSelection(QQuickItem *parent)
    : QQuickItem(parent)
{
#if QT_CONFIG(im)
    setFlags(ItemIsFocusScope | ItemAcceptsInputMethod);
#endif
}

/*!
    \internal
*/
QQuickPdfSelection::~QQuickPdfSelection() = default;

/*!
    \qmlproperty PdfDocument PdfSelection::document

    This property holds the PDF document in which to select text.
*/
QQuickPdfDocument *QQuickPdfSelection::document() const
{
    return m_document;
}

void QQuickPdfSelection::setDocument(QQuickPdfDocument *document)
{
    if (m_document == document)
        return;

    if (m_document) {
        disconnect(m_document, &QQuickPdfDocument::sourceChanged,
                   this, &QQuickPdfSelection::resetPoints);
    }
    m_document = document;
    emit documentChanged();
    resetPoints();
    connect(m_document, &QQuickPdfDocument::sourceChanged,
            this, &QQuickPdfSelection::resetPoints);
}

/*!
    \qmlproperty list<list<point>> PdfSelection::geometry

    A set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance to render a batch of
    rectangles around the text regions that are included in the selection:

    \qml
    PdfDocument {
        id: doc
    }
    PdfSelection {
        id: selection
        document: doc
        from: textSelectionDrag.centroid.pressPosition
        to: textSelectionDrag.centroid.position
        hold: !textSelectionDrag.active
    }
    Shape {
        ShapePath {
            PathMultiline {
                paths: selection.geometry
            }
        }
    }
    DragHandler {
        id: textSelectionDrag
        acceptedDevices: PointerDevice.Mouse | PointerDevice.Stylus
        target: null
    }
    \endqml

    \sa PathMultiline
*/
QList<QPolygonF> QQuickPdfSelection::geometry() const
{
    return m_geometry;
}

/*!
    \qmlmethod void PdfSelection::clear()

    Clears the current selection.
*/
void QQuickPdfSelection::clear()
{
    m_hitPoint = QPointF();
    m_from = QPointF();
    m_to = QPointF();
    m_heightAtAnchor = 0;
    m_heightAtCursor = 0;
    m_fromCharIndex = -1;
    m_toCharIndex = -1;
    m_text.clear();
    m_geometry.clear();
    emit fromChanged();
    emit toChanged();
    emit textChanged();
    emit selectedAreaChanged();
    QGuiApplication::inputMethod()->update(Qt::ImQueryInput);
}

/*!
    \qmlmethod void PdfSelection::selectAll()

    Selects all text on the current \l page.
*/
void QQuickPdfSelection::selectAll()
{
    if (!m_document)
        return;
    QPdfSelection sel = m_document->document()->getAllText(m_page);
    if (sel.text() != m_text) {
        m_text = sel.text();
        if (QGuiApplication::clipboard()->supportsSelection())
            sel.copyToClipboard(QClipboard::Selection);
        emit textChanged();
    }

    if (sel.bounds() != m_geometry) {
        m_geometry = sel.bounds();
        emit selectedAreaChanged();
    }
#if QT_CONFIG(im)
    m_fromCharIndex = sel.startIndex();
    m_toCharIndex = sel.endIndex();
    if (sel.bounds().isEmpty()) {
        m_from = QPointF();
        m_to = QPointF();
    } else {
        m_from = sel.bounds().first().boundingRect().topLeft() * m_renderScale;
        m_to = sel.bounds().last().boundingRect().bottomRight() * m_renderScale - QPointF(0, m_heightAtCursor);
    }

    QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle | Qt::ImAnchorRectangle);
#endif
}

#if QT_CONFIG(im)
void QQuickPdfSelection::keyReleaseEvent(QKeyEvent *ev)
{
    qCDebug(qLcIm) << "release" << ev;
    const auto &allText = pageText();
    if (ev == QKeySequence::MoveToPreviousWord) {
        if (!m_document)
            return;
        // iOS sends MoveToPreviousWord first to get to the beginning of the word,
        // and then SelectNextWord to select the whole word.
        int i = allText.lastIndexOf(WordDelimiter, m_fromCharIndex - allText.size());
        if (i < 0)
            i = 0;
        else
            i += 1; // don't select the space before the word
        auto sel = m_document->document()->getSelectionAtIndex(m_page, i, m_text.size() + m_fromCharIndex - i);
        update(sel);
        QGuiApplication::inputMethod()->update(Qt::ImAnchorRectangle);
    } else if (ev == QKeySequence::SelectNextWord) {
        if (!m_document)
            return;
        int i = allText.indexOf(WordDelimiter, m_toCharIndex);
        if (i < 0)
            i = allText.size(); // go to the end of m_textAfter
        auto sel = m_document->document()->getSelectionAtIndex(m_page, m_fromCharIndex, m_text.size() + i - m_toCharIndex);
        update(sel);
        QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle);
    } else if (ev == QKeySequence::Copy) {
        copyToClipboard();
    }
}

void QQuickPdfSelection::inputMethodEvent(QInputMethodEvent *event)
{
    for (auto attr : event->attributes()) {
        switch (attr.type) {
        case QInputMethodEvent::Cursor:
            qCDebug(qLcIm) << "QInputMethodEvent::Cursor: moved to" << attr.start << "len" << attr.length;
            break;
        case QInputMethodEvent::Selection: {
            if (!m_document)
                return;
            auto sel = m_document->document()->getSelectionAtIndex(m_page, attr.start, attr.length);
            update(sel);
            qCDebug(qLcIm) << "QInputMethodEvent::Selection: from" << attr.start << "len" << attr.length
                           << "result:" << m_fromCharIndex << "->" << m_toCharIndex << sel.boundingRectangle();
            // the iOS plugin decided that it wanted to change the selection, but still has to be told to move the handles (!?)
            QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle | Qt::ImAnchorRectangle);
            break;
        }
        case QInputMethodEvent::Language:
        case QInputMethodEvent::Ruby:
        case QInputMethodEvent::TextFormat:
            break;
        }
    }
}

QVariant QQuickPdfSelection::inputMethodQuery(Qt::InputMethodQuery query, const QVariant &argument) const
{
    if (!argument.isNull()) {
        qCDebug(qLcIm) << "IM query" << query << "with arg" << argument;
        if (query == Qt::ImCursorPosition) {
            if (!m_document)
                return {};
            // If it didn't move since last time, return the same result.
            if (m_hitPoint == argument.toPointF())
                return inputMethodQuery(query);
            m_hitPoint = argument.toPointF();
            auto tp = m_document->document()->d->hitTest(m_page, m_hitPoint / m_renderScale);
            qCDebug(qLcIm) << "ImCursorPosition hit testing in px" << m_hitPoint << "pt" << (m_hitPoint / m_renderScale)
                           << "got char index" << tp.charIndex << "@" << tp.position << "pt," << tp.position * m_renderScale << "px";
            if (tp.charIndex >= 0) {
                m_toCharIndex = tp.charIndex;
                m_to = tp.position * m_renderScale - QPointF(0, m_heightAtCursor);
                m_heightAtCursor = tp.height * m_renderScale;
                if (qFuzzyIsNull(m_heightAtAnchor))
                    m_heightAtAnchor = m_heightAtCursor;
            }
        }
    }
    return inputMethodQuery(query);
}

QVariant QQuickPdfSelection::inputMethodQuery(Qt::InputMethodQuery query) const
{
    QVariant ret;
    switch (query) {
    case Qt::ImEnabled:
        ret = true;
        break;
    case Qt::ImHints:
        ret = QVariant(Qt::ImhMultiLine | Qt::ImhNoPredictiveText);
        break;
    case Qt::ImInputItemClipRectangle:
        ret = boundingRect();
        break;
    case Qt::ImAnchorPosition:
        ret = m_fromCharIndex;
        break;
    case Qt::ImAbsolutePosition:
        ret = m_toCharIndex;
        break;
    case Qt::ImCursorPosition:
        ret = m_toCharIndex;
        break;
    case Qt::ImAnchorRectangle:
        ret = QRectF(m_from, QSizeF(1, m_heightAtAnchor));
        break;
    case Qt::ImCursorRectangle:
        ret = QRectF(m_to, QSizeF(1, m_heightAtCursor));
        break;
    case Qt::ImSurroundingText:
        ret = QVariant(pageText());
        break;
    case Qt::ImTextBeforeCursor:
        ret = QVariant(pageText().mid(0, m_toCharIndex));
        break;
    case Qt::ImTextAfterCursor:
        ret = QVariant(pageText().mid(m_toCharIndex));
        break;
    case Qt::ImCurrentSelection:
        ret = QVariant(m_text);
        break;
    case Qt::ImEnterKeyType:
        break;
    case Qt::ImFont: {
        QFont font = QGuiApplication::font();
        font.setPointSizeF(m_heightAtCursor);
        ret = font;
        break;
    }
    case Qt::ImMaximumTextLength:
        break;
    case Qt::ImPreferredLanguage:
        break;
    case Qt::ImPlatformData:
        break;
    case Qt::ImReadOnly:
        ret = true;
        break;
    case Qt::ImQueryInput:
    case Qt::ImQueryAll:
        qWarning() << "unexpected composite query";
        break;
    }
    qCDebug(qLcIm) << "IM query" << query << "returns" << ret;
    return ret;
}
#endif // QT_CONFIG(im)

const QString &QQuickPdfSelection::pageText() const
{
    if (m_pageTextDirty) {
        if (!m_document)
            return m_pageText;
        m_pageText = m_document->document()->getAllText(m_page).text();
        m_pageTextDirty = false;
    }
    return m_pageText;
}

void QQuickPdfSelection::resetPoints()
{
    bool wasHolding = m_hold;
    m_hold = false;
    setFrom(QPointF());
    setTo(QPointF());
    m_hold = wasHolding;
}

/*!
    \qmlproperty int PdfSelection::page

    The page number on which to search.

    \sa QtQuick::Image::currentFrame
*/
int QQuickPdfSelection::page() const
{
    return m_page;
}

void QQuickPdfSelection::setPage(int page)
{
    if (m_page == page)
        return;

    m_page = page;
    m_pageTextDirty = true;
    emit pageChanged();
    resetPoints();
}

/*!
    \qmlproperty real PdfSelection::renderScale
    \brief The ratio from points to pixels at which the page is rendered.

    This is used to scale \l from and \l to to find ranges of
    selected characters in the document, because positions within the document
    are always given in points.
*/
qreal QQuickPdfSelection::renderScale() const
{
    return m_renderScale;
}

void QQuickPdfSelection::setRenderScale(qreal scale)
{
    if (qFuzzyIsNull(scale)) {
        qWarning() << "PdfSelection.renderScale cannot be set to 0.";
        return;
    }

    if (qFuzzyCompare(scale, m_renderScale))
        return;

    m_renderScale = scale;
    emit renderScaleChanged();
    updateResults();
}

/*!
    \qmlproperty point PdfSelection::from

    The beginning location, in pixels from the upper-left corner of the page,
    from which to find selected text. This can be bound to the
    \c centroid.pressPosition of a \l DragHandler to begin selecting text from
    the position where the user presses the mouse button and begins dragging,
    for example.
*/
QPointF QQuickPdfSelection::from() const
{
    return m_from;
}

void QQuickPdfSelection::setFrom(QPointF from)
{
    if (m_hold || m_from == from)
        return;

    m_from = from;
    emit fromChanged();
    updateResults();
}

/*!
    \qmlproperty point PdfSelection::to

    The ending location, in pixels from the upper-left corner of the page,
    from which to find selected text. This can be bound to the
    \c centroid.position of a \l DragHandler to end selection of text at the
    position where the user is currently dragging the mouse, for example.
*/
QPointF QQuickPdfSelection::to() const
{
    return m_to;
}

void QQuickPdfSelection::setTo(QPointF to)
{
    if (m_hold || m_to == to)
        return;

    m_to = to;
    emit toChanged();
    updateResults();
}

/*!
    \qmlproperty bool PdfSelection::hold

    Controls whether to hold the existing selection regardless of changes to
    \l from and \l to. This property can be set to \c true when the mouse
    or touchpoint is released, so that the selection is not lost due to the
    point bindings changing.
*/
bool QQuickPdfSelection::hold() const
{
    return m_hold;
}

void QQuickPdfSelection::setHold(bool hold)
{
    if (m_hold == hold)
        return;

    m_hold = hold;
    emit holdChanged();
}

/*!
    \qmlproperty string PdfSelection::string

    The string found.
*/
QString QQuickPdfSelection::text() const
{
    return m_text;
}

#if QT_CONFIG(clipboard)
/*!
    \qmlmethod void PdfSelection::copyToClipboard()

    Copies plain text from the \l string property to the system clipboard.
*/
void QQuickPdfSelection::copyToClipboard() const
{
     QGuiApplication::clipboard()->setText(m_text);
}
#endif

void QQuickPdfSelection::updateResults()
{
    if (!m_document)
        return;
    QPdfSelection sel = m_document->document()->getSelection(m_page,
            m_from / m_renderScale, m_to / m_renderScale);
    update(sel, true);
}

void QQuickPdfSelection::update(const QPdfSelection &sel, bool textAndGeometryOnly)
{
    if (sel.text() != m_text) {
        m_text = sel.text();
        if (QGuiApplication::clipboard()->supportsSelection())
            sel.copyToClipboard(QClipboard::Selection);
        emit textChanged();
    }

    if (sel.bounds() != m_geometry) {
        m_geometry = sel.bounds();
        emit selectedAreaChanged();
    }

    if (textAndGeometryOnly)
        return;

    m_fromCharIndex = sel.startIndex();
    m_toCharIndex = sel.endIndex();
    if (sel.bounds().isEmpty()) {
        m_from = sel.boundingRectangle().topLeft() * m_renderScale;
        m_to = m_from;
    } else {
        Qt::InputMethodQueries toUpdate = {};
        QRectF firstLineBounds = sel.bounds().first().boundingRect();
        m_from = firstLineBounds.topLeft() * m_renderScale;
        if (!qFuzzyCompare(m_heightAtAnchor, firstLineBounds.height())) {
            m_heightAtAnchor = firstLineBounds.height() * m_renderScale;
            toUpdate.setFlag(Qt::ImAnchorRectangle);
        }
        QRectF lastLineBounds = sel.bounds().last().boundingRect();
        if (!qFuzzyCompare(m_heightAtCursor, lastLineBounds.height())) {
            m_heightAtCursor = lastLineBounds.height() * m_renderScale;
            toUpdate.setFlag(Qt::ImCursorRectangle);
        }
        m_to = lastLineBounds.topRight() * m_renderScale;
        if (toUpdate)
            QGuiApplication::inputMethod()->update(toUpdate);
    }
}

QT_END_NAMESPACE

#include "moc_qquickpdfselection_p.cpp"
