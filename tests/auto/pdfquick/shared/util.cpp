// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "util.h"
#include <QtQuick/QQuickItem>

QQuickDataTest::QQuickDataTest() :
    m_initialized(false),
#ifdef QT_TESTCASE_BUILDDIR
    m_dataDirectory(QTest::qFindTestData("data", QT_QMLTEST_DATADIR, 0, QT_TESTCASE_BUILDDIR)),
#else
    m_dataDirectory(QTest::qFindTestData("data", QT_QMLTEST_DATADIR, 0)),
#endif

    m_dataDirectoryUrl(m_dataDirectory.startsWith(QLatin1Char(':'))
        ? QUrl(QLatin1String("qrc") + m_dataDirectory)
        : QUrl::fromLocalFile(m_dataDirectory + QLatin1Char('/')))
{
}

QQuickDataTest::~QQuickDataTest()
{
}

void QQuickDataTest::initTestCase()
{
    QVERIFY2(!m_dataDirectory.isEmpty(), "'data' directory not found");
    m_directory = QFileInfo(m_dataDirectory).absolutePath();
    if (m_dataDirectoryUrl.scheme() != QLatin1String("qrc"))
        QVERIFY2(QDir::setCurrent(m_directory), qPrintable(QLatin1String("Could not chdir to ") + m_directory));

    if (QGuiApplication::platformName() == QLatin1String("offscreen")
        || QGuiApplication::platformName() == QLatin1String("minimal"))
    {
        QSKIP("Skipping visual tests due to running with offscreen/minimal");
    }

    m_initialized = true;
}

void QQuickDataTest::cleanupTestCase()
{
    m_initialized = false;
}

QString QQuickDataTest::testFile(const QString &fileName) const
{
    if (m_directory.isEmpty())
        qFatal("QQuickDataTest::initTestCase() not called.");
    QString result = m_dataDirectory;
    result += QLatin1Char('/');
    result += fileName;
    return result;
}

QObject *QQuickDataTest::findFirstChild(QObject *parent, const char *className)
{
    const auto children = parent->findChildren<QObject*>();
    for (QObject *child : children) {
        if (child->inherits(className))
            return child;
    }
    return nullptr;
}

bool QQuickDataTest::showView(QQuickView &view, const QUrl &url)
{
    view.setSource(url);
    while (view.status() == QQuickView::Loading)
        QTest::qWait(10);
    if (view.status() != QQuickView::Ready)
        return false;
    const QRect screenGeometry = view.screen()->availableGeometry();
    const QSize size = view.size();
    const QPoint offset = QPoint(size.width() / 2, size.height() / 2);
    view.setFramePosition(screenGeometry.center() - offset);
#if QT_CONFIG(cursor) // Get the cursor out of the way.
     QCursor::setPos(view.geometry().topRight() + QPoint(100, 100));
#endif
    view.show();
    if (!QTest::qWaitForWindowExposed(&view))
        return false;
    if (!view.rootObject())
        return false;
    return true;
}

QQuickItem *QQuickDataTest::repeaterItemAt(QQuickItem *repeater, int i)
{
    static const QMetaMethod itemAtMethod = repeater->metaObject()->method(
                repeater->metaObject()->indexOfMethod("itemAt(int)"));
    QQuickItem *ret = nullptr;
    itemAtMethod.invoke(repeater, Qt::DirectConnection, Q_RETURN_ARG(QQuickItem*, ret), Q_ARG(int, i));
    return ret;
}

QQuickItem *QQuickDataTest::tableViewItemAtCell(QQuickItem *table, int col, int row)
{
    static const QMetaMethod itemAtCellMethod = table->metaObject()->method(
                table->metaObject()->indexOfMethod("itemAtCell(int,int)"));
    QQuickItem *ret = nullptr;
    itemAtCellMethod.invoke(table, Qt::DirectConnection,
                            Q_RETURN_ARG(QQuickItem*, ret), Q_ARG(int, col), Q_ARG(int, row));
    return ret;
}

QPoint QQuickDataTest::tableViewContentPos(QQuickItem *table)
{
    return QPoint(table->property("contentX").toInt(), table->property("contentY").toInt());
}
