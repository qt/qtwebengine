// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QBuffer>
#include <QByteArray>
#include <QFont>
#include <QPdfDocument>
#include <QtGlobal>

#include <cstdint>

// silence warnings
static QtMessageHandler mh = qInstallMessageHandler([](QtMsgType, const QMessageLogContext &,
                                                       const QString &) {});

// Exercises QPdfDocument's load()/close() state machine across repeated
// cycles on the same instance, rather than a single load() on a fresh
// object each run: PDFium's own fuzzers call the C API directly and never
// touch this lifecycle, so it's the part of the wrapper they can't cover.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    static int argc = 3;
    static char arg1[] = "fuzzer";
    static char arg2[] = "-platform";
    static char arg3[] = "minimal";
    static char *argv[] = {arg1, arg2, arg3, nullptr};
    static QGuiApplication qga(argc, argv);

    if (Size < 1)
        return 0;

    // The first byte selects, per cycle, whether to call close() after
    // load(); the rest of the input is sliced into chunks, one per cycle.
    const quint8 plan = Data[0];
    const char *rest = reinterpret_cast<const char *>(Data + 1);
    const size_t restSize = Size - 1;

    constexpr int kCycles = 4;
    const size_t chunkSize = restSize / kCycles;

    QPdfDocument doc;

    for (int i = 0; i < kCycles; ++i) {
        const size_t offset = i * chunkSize;
        const size_t len = (i == kCycles - 1) ? (restSize - offset) : chunkSize;
        QByteArray chunk = QByteArray::fromRawData(rest + offset, static_cast<qsizetype>(len));
        QBuffer buf(&chunk);

        doc.load(&buf);

        if (plan & (1 << i))
            doc.close();
    }

    doc.close();
    QFont::cleanup(); // avoid memleaks, QTBUG-140076
    return 0;
}
