// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFSEARCHMODEL_P_H
#define QQUICKPDFSEARCHMODEL_P_H

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

#include <QtPdf/qpdfsearchmodel.h>
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class  Q_PDFQUICK_EXPORT QQuickPdfSearchModel : public QPdfSearchModel
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int currentResult READ currentResult WRITE setCurrentResult NOTIFY currentResultChanged)
    Q_PROPERTY(QPdfLink currentResultLink READ currentResultLink NOTIFY currentResultLinkChanged)
    Q_PROPERTY(QList<QPolygonF> currentPageBoundingPolygons READ currentPageBoundingPolygons NOTIFY currentPageBoundingPolygonsChanged)
    Q_PROPERTY(QList<QPolygonF> currentResultBoundingPolygons READ currentResultBoundingPolygons NOTIFY currentResultBoundingPolygonsChanged)
    Q_PROPERTY(QRectF currentResultBoundingRect READ currentResultBoundingRect NOTIFY currentResultBoundingRectChanged)
    QML_NAMED_ELEMENT(PdfSearchModel)
    QML_ADDED_IN_VERSION(5, 15)

public:
    explicit QQuickPdfSearchModel(QObject *parent = nullptr);
    ~QQuickPdfSearchModel() override;

    QQuickPdfDocument *document() const;
    void setDocument(QQuickPdfDocument * document);

    Q_INVOKABLE QList<QPolygonF> boundingPolygonsOnPage(int page);

    int currentPage() const { return m_currentPage; }
    void setCurrentPage(int currentPage);

    int currentResult() const { return m_currentResult; }
    void setCurrentResult(int currentResult);

    QPdfLink currentResultLink() const;
    QList<QPolygonF> currentPageBoundingPolygons() const;
    QList<QPolygonF> currentResultBoundingPolygons() const;
    QRectF currentResultBoundingRect() const;

signals:
    void currentPageChanged();
    void currentResultChanged();
    void currentResultLinkChanged();
    void currentPageBoundingPolygonsChanged();
    void currentResultBoundingPolygonsChanged();
    void currentResultBoundingRectChanged();

private:
    void updateResults();
    void onResultsChanged();

private:
    QQuickPdfDocument *m_quickDocument = nullptr;
    int m_currentPage = 0;
    int m_currentResult = 0;
    bool m_suspendSignals = false;

    Q_DISABLE_COPY(QQuickPdfSearchModel)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfSearchModel)
QML_DECLARE_TYPE(QPdfSelection)

#endif // QQUICKPDFSEARCHMODEL_P_H
