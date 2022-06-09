// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFSELECTION_P_H
#define QQUICKPDFSELECTION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtPdfQuick/private/qtpdfquickglobal_p.h>
#include <QtPdfQuick/private/qquickpdfdocument_p.h>

#include <QtCore/QPointF>
#include <QtCore/QVariant>
#include <QtGui/QPolygonF>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>

QT_BEGIN_NAMESPACE
class QPdfSelection;

class Q_PDFQUICK_EXPORT QQuickPdfSelection : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(qreal renderScale READ renderScale WRITE setRenderScale NOTIFY renderScaleChanged)
    Q_PROPERTY(QPointF from READ from WRITE setFrom NOTIFY fromChanged)
    Q_PROPERTY(QPointF to READ to WRITE setTo NOTIFY toChanged)
    Q_PROPERTY(bool hold READ hold WRITE setHold NOTIFY holdChanged)

    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QList<QPolygonF> geometry READ geometry NOTIFY selectedAreaChanged)
    QML_NAMED_ELEMENT(PdfSelection)
    QML_ADDED_IN_VERSION(5, 15)

public:
    explicit QQuickPdfSelection(QQuickItem *parent = nullptr);
    ~QQuickPdfSelection() override;

    QQuickPdfDocument *document() const;
    void setDocument(QQuickPdfDocument * document);
    int page() const;
    void setPage(int page);
    qreal renderScale() const;
    void setRenderScale(qreal scale);
    QPointF from() const;
    void setFrom(QPointF from);
    QPointF to() const;
    void setTo(QPointF to);
    bool hold() const;
    void setHold(bool hold);

    QString text() const;
    QList<QPolygonF> geometry() const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void selectAll();
#if QT_CONFIG(clipboard)
    Q_INVOKABLE void copyToClipboard() const;
#endif

signals:
    void documentChanged();
    void pageChanged();
    void renderScaleChanged();
    void fromChanged();
    void toChanged();
    void holdChanged();
    void textChanged();
    void selectedAreaChanged();

protected:
#if QT_CONFIG(im)
    void keyReleaseEvent(QKeyEvent *ev) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    Q_INVOKABLE QVariant inputMethodQuery(Qt::InputMethodQuery query, const QVariant &argument) const;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
#endif

private:
    void resetPoints();
    void updateResults();
    void update(const QPdfSelection &sel, bool textAndGeometryOnly = false);
    const QString &pageText() const;

private:
    QQuickPdfDocument *m_document = nullptr;
    mutable QPointF m_hitPoint;
    QPointF m_from;
    mutable QPointF m_to;
    qreal m_renderScale = 1;
    mutable qreal m_heightAtAnchor = 0;
    mutable qreal m_heightAtCursor = 0;
    QString m_text;             // selected text
    mutable QString m_pageText; // all text on the page
    QList<QPolygonF> m_geometry;
    int m_page = 0;
    int m_fromCharIndex = -1;   // same as anchor position
    mutable int m_toCharIndex = -1; // same as cursor position
    bool m_hold = false;
    mutable bool m_pageTextDirty = true;

    Q_DISABLE_COPY(QQuickPdfSelection)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfSelection)

#endif // QQUICKPDFSELECTION_P_H
