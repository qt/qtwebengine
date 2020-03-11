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

#include <QPointF>
#include <QPolygonF>
#include <QVariant>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuickPdfDocument;

class QQuickPdfSelection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(QPointF fromPoint READ fromPoint WRITE setFromPoint NOTIFY fromPointChanged)
    Q_PROPERTY(QPointF toPoint READ toPoint WRITE setToPoint NOTIFY toPointChanged)
    Q_PROPERTY(bool hold READ hold WRITE setHold NOTIFY holdChanged)

    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QVector<QPolygonF> geometry READ geometry NOTIFY geometryChanged)

public:
    explicit QQuickPdfSelection(QObject *parent = nullptr);

    QQuickPdfDocument *document() const;
    void setDocument(QQuickPdfDocument * document);
    int page() const;
    void setPage(int page);
    QPointF fromPoint() const;
    void setFromPoint(QPointF fromPoint);
    QPointF toPoint() const;
    void setToPoint(QPointF toPoint);
    bool hold() const;
    void setHold(bool hold);

    QString text() const;
    QVector<QPolygonF> geometry() const;

#if QT_CONFIG(clipboard)
    Q_INVOKABLE void copyToClipboard() const;
#endif

signals:
    void documentChanged();
    void pageChanged();
    void fromPointChanged();
    void toPointChanged();
    void holdChanged();
    void textChanged();
    void geometryChanged();

private:
    void resetPoints();
    void updateResults();

private:
    QQuickPdfDocument *m_document = nullptr;
    QPointF m_fromPoint;
    QPointF m_toPoint;
    QString m_text;
    QVector<QPolygonF> m_geometry;
    int m_page = 0;
    bool m_hold = false;

    Q_DISABLE_COPY(QQuickPdfSelection)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfSelection)
\
#endif // QQUICKPDFSELECTION_P_H
