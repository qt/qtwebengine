// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QRegularExpression>
#include <QSignalSpy>
#include <QTest>
#include <QtQuick/QQuickView>
#include <QtPdfQuick/private/qquickpdfdocument_p.h>
#include <QtPdfQuick/private/qquickpdfpageimage_p.h>
#include "../shared/util.h"

using namespace Qt::StringLiterals;

// #define DEBUG_WRITE_OUTPUT

#ifdef DEBUG_WRITE_OUTPUT
Q_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")
#endif

class tst_PdfPageImage : public QQuickDataTest
{
    Q_OBJECT

private Q_SLOTS:
    void settableProperties_data();
    void settableProperties();

public:
    enum Property {
        Source = 0x01,
        Document = 0x02,
        SourceSize = 0x04,
        SourceClipRect = 0x08,
        MipMap = 0x10,
        AutoTransform = 0x20,
        Asynchronous = 0x40,
        NoCache = 0x80,
        Mirror = 0x100,
        MirrorVertically = 0x200,
        ColorSpace = 0x400,
    };
    Q_DECLARE_FLAGS(Properties, Property)
    Q_FLAG(Properties)

private:
#ifdef DEBUG_WRITE_OUTPUT
    QTemporaryDir m_tmpDir;
#endif
};

void tst_PdfPageImage::settableProperties_data()
{
    QTest::addColumn<tst_PdfPageImage::Properties>("toSet");
    QTest::addColumn<QSize>("expectedSize");
    QTest::addColumn<QRegularExpression>("expectedWarning");

    const QRegularExpression NoWarning;
    const qreal dpr = qGuiApp->devicePixelRatio();

    QTest::newRow("source") << Properties(Source) << (QSizeF(600, 790) * dpr).toSize()
        << QRegularExpression("document property not set: falling back to inefficient loading"); // QTBUG-104767
    QTest::newRow("document") << Properties(Document) << QSize(600, 790) << NoWarning;
    QTest::newRow("source and document") << Properties(Source | Document) << QSize(600, 790)
        << QRegularExpression("document and source properties in conflict");
    QTest::newRow("document and sourceSize") << Properties(Document | SourceSize) << QSize(100, 100) << NoWarning;
    QTest::newRow("document and sourceClipRect") << Properties(Document | SourceClipRect) << QSize(100, 100) << NoWarning;
    QTest::newRow("document and autoTransform") << Properties(Document | AutoTransform) << QSize(600, 790) << NoWarning;
    QTest::newRow("document and async") << Properties(Document | Asynchronous) << QSize(600, 790) << NoWarning;
    QTest::newRow("document and nocache") << Properties(Document | NoCache) << QSize(600, 790) << NoWarning;
    QTest::newRow("document and mirror") << Properties(Document | Mirror) << QSize(600, 790) << NoWarning;
    QTest::newRow("document and mirrorVertically") << Properties(Document | MirrorVertically) << QSize(600, 790) << NoWarning;
    QTest::newRow("document and colorSpace") << Properties(Document | ColorSpace) << QSize(600, 790) << NoWarning;
}

void tst_PdfPageImage::settableProperties()
{
    QFETCH(tst_PdfPageImage::Properties, toSet);
    QFETCH(QSize, expectedSize);
    QFETCH(QRegularExpression, expectedWarning);

    QQuickView window;
    if (!expectedWarning.pattern().isEmpty())
        QTest::ignoreMessage(QtWarningMsg, expectedWarning);
    QVERIFY(showView(window, testFileUrl("pdfPageImage.qml")));
    QQuickPdfPageImage *pdfImage = window.rootObject()->findChild<QQuickPdfPageImage *>();
    QVERIFY(pdfImage);
    QQuickPdfDocument *doc = window.rootObject()->findChild<QQuickPdfDocument *>();
    QVERIFY(doc);
    if (toSet.testFlag(Document))
        pdfImage->setDocument(doc);
    if (toSet.testFlag(Source))
        pdfImage->setSource(doc->source());
    if (toSet.testFlag(SourceSize))
        pdfImage->setSourceSize({100, 100});
    if (toSet.testFlag(SourceClipRect))
        pdfImage->setSourceClipRect({100, 100, 100, 100});
    if (toSet.testFlag(MipMap))
        pdfImage->setMipmap(true);
    if (toSet.testFlag(AutoTransform))
        pdfImage->setAutoTransform(true);
    if (toSet.testFlag(Asynchronous)) {
        QCOMPARE(pdfImage->asynchronous(), false);
        // test the opposite of the default
        pdfImage->setAsynchronous(true);
    }
    if (toSet.testFlag(NoCache)) {
        QCOMPARE(pdfImage->cache(), true);
        // test the opposite of the default
        pdfImage->setCache(false);
    }
    if (toSet.testFlag(Mirror)) {
        QCOMPARE(pdfImage->mirror(), false);
        pdfImage->setMirror(true);
    }
    if (toSet.testFlag(MirrorVertically)) {
        QCOMPARE(pdfImage->mirrorVertically(), false);
        pdfImage->setMirrorVertically(true);
    }
    if (toSet.testFlag(ColorSpace))
        pdfImage->setColorSpace(QColorSpace::ProPhotoRgb);
    QTRY_COMPARE(pdfImage->status(), QQuickPdfPageImage::Ready);
    const QImage img = pdfImage->image();
    QCOMPARE(img.size(), expectedSize);
#ifdef DEBUG_WRITE_OUTPUT
    m_tmpDir.setAutoRemove(false);
    const auto path = m_tmpDir.filePath(QString::fromLocal8Bit(QTest::currentDataTag()) + ".png");
    qCDebug(lcTests) << "saving to" << path;
    img.save(path);
#endif
}

QTEST_MAIN(tst_PdfPageImage)
#include "tst_pdfpageimage.moc"
