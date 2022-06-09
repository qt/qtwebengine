// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFPAGERENDERER_H
#define QPDFPAGERENDERER_H

#include <QtPdf/qtpdfglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtPdf/qpdfdocument.h>
#include <QtPdf/qpdfdocumentrenderoptions.h>

QT_BEGIN_NAMESPACE

class QPdfPageRendererPrivate;

class Q_PDF_EXPORT QPdfPageRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(RenderMode renderMode READ renderMode WRITE setRenderMode NOTIFY renderModeChanged)

public:
    enum class RenderMode
    {
        MultiThreaded,
        SingleThreaded
    };
    Q_ENUM(RenderMode)

    QPdfPageRenderer() : QPdfPageRenderer(nullptr) {}
    explicit QPdfPageRenderer(QObject *parent);
    ~QPdfPageRenderer() override;

    RenderMode renderMode() const;
    void setRenderMode(RenderMode mode);

    QPdfDocument* document() const;
    void setDocument(QPdfDocument *document);

    quint64 requestPage(int pageNumber, QSize imageSize,
                        QPdfDocumentRenderOptions options = QPdfDocumentRenderOptions());

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);
    void renderModeChanged(QPdfPageRenderer::RenderMode renderMode);

    void pageRendered(int pageNumber, QSize imageSize, const QImage &image,
                      QPdfDocumentRenderOptions options, quint64 requestId);

private:
    QScopedPointer<QPdfPageRendererPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif
