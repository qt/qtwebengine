// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QUICK_VISUAL_TEST_UTIL_H
#define QUICK_VISUAL_TEST_UTIL_H

#include <QtCore/QUrl>
#include <QtQuick/QQuickView>
#include <QtTest/QTest>

/*! \internal
    Base class for tests with data that are located in a "data" subfolder.
*/
class QQuickDataTest : public QObject
{
    Q_OBJECT
public:
    QQuickDataTest();
    ~QQuickDataTest();

    bool initialized() const { return m_initialized; }

    bool showView(QQuickView &view, const QUrl &url);

    QString testFile(const QString &fileName) const;
    inline QString testFile(const char *fileName) const
        { return testFile(QLatin1String(fileName)); }
    inline QUrl testFileUrl(const QString &fileName) const
        {
            const QString fn = testFile(fileName);
            return fn.startsWith(QLatin1Char(':'))
                ? QUrl(QLatin1String("qrc") + fn)
                : QUrl::fromLocalFile(fn);
        }
    inline QUrl testFileUrl(const char *fileName) const
        { return testFileUrl(QLatin1String(fileName)); }

    inline QString dataDirectory() const { return m_dataDirectory; }
    inline QUrl dataDirectoryUrl() const { return m_dataDirectoryUrl; }
    inline QString directory() const  { return m_directory; }

    QObject *findFirstChild(QObject *parent, const char *className);
    QQuickItem *repeaterItemAt(QQuickItem *repeater, int i);
    QQuickItem *tableViewItemAtCell(QQuickItem *table, int col, int row);
    QPoint tableViewContentPos(QQuickItem *table);

public slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();

private:
    bool m_initialized;
    QString m_dataDirectory;
    QUrl m_dataDirectoryUrl;
    QString m_directory;
};

#endif
