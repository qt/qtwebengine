// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QPdfDocument>
#include <QPdfPageRenderer>

#include <QtTest/QtTest>

class tst_QPdfPageRenderer: public QObject
{
    Q_OBJECT

private slots:
    void defaultValues();
    void withNoDocument();
    void withEmptyDocument();
    void withLoadedDocumentSingleThreaded();
    void withLoadedDocumentMultiThreaded();
    void switchingRenderMode();
};

void tst_QPdfPageRenderer::defaultValues()
{
    QPdfPageRenderer pageRenderer;

    QCOMPARE(pageRenderer.document(), nullptr);
    QCOMPARE(pageRenderer.renderMode(), QPdfPageRenderer::RenderMode::SingleThreaded);
}

void tst_QPdfPageRenderer::withNoDocument()
{
    QPdfPageRenderer pageRenderer;

    const QSize imageSize(100, 100);
    const quint64 requestId = pageRenderer.requestPage(0, imageSize);

    QCOMPARE(requestId, quint64(0));
}

void tst_QPdfPageRenderer::withEmptyDocument()
{
    QPdfDocument document;
    QPdfPageRenderer pageRenderer;

    pageRenderer.setDocument(&document);

    const QSize imageSize(100, 100);
    const quint64 requestId = pageRenderer.requestPage(0, imageSize);

    QCOMPARE(requestId, quint64(0));
}

void tst_QPdfPageRenderer::withLoadedDocumentSingleThreaded()
{
    QPdfDocument document;
    QPdfPageRenderer pageRenderer;
    pageRenderer.setDocument(&document);

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagerenderer.pdf")), QPdfDocument::Error::None);

    QSignalSpy pageRenderedSpy(&pageRenderer, &QPdfPageRenderer::pageRendered);

    const QSize imageSize(100, 100);
    const quint64 requestId = pageRenderer.requestPage(0, imageSize);

    QCOMPARE(requestId, quint64(1));
    QTRY_COMPARE(pageRenderedSpy.size(), 1);
    QCOMPARE(pageRenderedSpy[0][0].toInt(), 0);
    QCOMPARE(pageRenderedSpy[0][1].toSize(), imageSize);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>().size(), imageSize);
    QCOMPARE(pageRenderedSpy[0][4].toULongLong(), requestId);
}

void tst_QPdfPageRenderer::withLoadedDocumentMultiThreaded()
{
    QPdfDocument document;

    QPdfPageRenderer pageRenderer;
    pageRenderer.setDocument(&document);
    pageRenderer.setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagerenderer.pdf")), QPdfDocument::Error::None);

    QSignalSpy pageRenderedSpy(&pageRenderer, &QPdfPageRenderer::pageRendered);

    const QSize imageSize(100, 100);
    const quint64 requestId = pageRenderer.requestPage(0, imageSize);

    QCOMPARE(requestId, quint64(1));
    QTRY_COMPARE(pageRenderedSpy.size(), 1);
    QCOMPARE(pageRenderedSpy[0][0].toInt(), 0);
    QCOMPARE(pageRenderedSpy[0][1].toSize(), imageSize);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>().size(), imageSize);
    QCOMPARE(pageRenderedSpy[0][4].toULongLong(), requestId);
}

void tst_QPdfPageRenderer::switchingRenderMode()
{
    QPdfDocument document;
    QPdfPageRenderer pageRenderer;
    pageRenderer.setDocument(&document);

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagerenderer.pdf")), QPdfDocument::Error::None);

    QSignalSpy pageRenderedSpy(&pageRenderer, &QPdfPageRenderer::pageRendered);

    // render single threaded
    const QSize imageSize(100, 100);
    const quint64 firstRequestId = pageRenderer.requestPage(0, imageSize);

    QTRY_COMPARE(pageRenderedSpy.size(), 1);
    QCOMPARE(pageRenderedSpy[0][0].toInt(), 0);
    QCOMPARE(pageRenderedSpy[0][1].toSize(), imageSize);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>().size(), imageSize);
    QCOMPARE(pageRenderedSpy[0][4].toULongLong(), firstRequestId);

    const QImage image = pageRenderedSpy[0][2].value<QImage>();

    pageRenderedSpy.clear();

    // switch to multi threaded
    pageRenderer.setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);

    const quint64 secondRequestId = pageRenderer.requestPage(0, imageSize);

    QVERIFY(firstRequestId != secondRequestId);
    QTRY_COMPARE(pageRenderedSpy.size(), 1);
    QCOMPARE(pageRenderedSpy[0][0].toInt(), 0);
    QCOMPARE(pageRenderedSpy[0][1].toSize(), imageSize);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>(), image);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>().size(), imageSize);
    QCOMPARE(pageRenderedSpy[0][4].toULongLong(), secondRequestId);

    pageRenderedSpy.clear();

    // switch back to single threaded
    pageRenderer.setRenderMode(QPdfPageRenderer::RenderMode::SingleThreaded);

    const quint64 thirdRequestId = pageRenderer.requestPage(0, imageSize);

    QTRY_COMPARE(pageRenderedSpy.size(), 1);
    QCOMPARE(pageRenderedSpy[0][0].toInt(), 0);
    QCOMPARE(pageRenderedSpy[0][1].toSize(), imageSize);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>(), image);
    QCOMPARE(pageRenderedSpy[0][2].value<QImage>().size(), imageSize);
    QCOMPARE(pageRenderedSpy[0][4].toULongLong(), thirdRequestId);
}

QTEST_MAIN(tst_QPdfPageRenderer)

#include "tst_qpdfpagerenderer.moc"
