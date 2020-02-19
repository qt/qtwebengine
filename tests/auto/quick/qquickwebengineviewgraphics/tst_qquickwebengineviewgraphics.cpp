/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "util.h"

#include <QtTest/QtTest>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickItem>
#include <QPainter>
#include <qtwebengineglobal.h>
#include <private/qquickwebengineview_p.h>

#include <map>

class TestView : public QQuickView {
    Q_OBJECT
public:
    TestView()
    {
        connect(this, &TestView::_q_exposeChanged, this, &TestView::exposeChanged,
                Qt::QueuedConnection);
    }

    virtual void exposeEvent(QExposeEvent *e) override {
        QQuickView::exposeEvent(e);
        emit _q_exposeChanged();
    }

Q_SIGNALS:
    void _q_exposeChanged();
    void exposeChanged();
};

class tst_QQuickWebEngineViewGraphics : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void simpleGraphics();
    void showHideShow();
    void simpleAcceleratedLayer();
    void reparentToOtherWindow();

private:
    void setHtml(const QString &html);
    QScopedPointer<TestView> m_view{new TestView};
};

static const QString greenSquare("<div style=\"background-color: #00ff00; position:absolute; left:50px; top: 50px; width: 50px; height: 50px;\"></div>");
static const QString acLayerGreenSquare("<div style=\"background-color: #00ff00; position:absolute; left:50px; top: 50px; width: 50px; height: 50px; transform: translateZ(0); -webkit-transform: translateZ(0);\"></div>");

static QImage makeGreenSquare(QImage::Format format)
{
    QImage image(150, 150, format);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.fillRect(50, 50, 50, 50, QColor("#00ff00"));
    return image;
}

static QImage getGreenSquare(QImage::Format format)
{
    static std::map<QImage::Format, QImage> images;
    auto it = images.find(format);
    if (it == images.end())
        it = images.emplace(format, makeGreenSquare(format)).first;
    return it->second;
}

static void verifyGreenSquare(QQuickWindow *window)
{
    QImage actual, expected;
    bool ok = QTest::qWaitFor([&](){
        actual = window->grabWindow();
        expected = getGreenSquare(actual.format());
        return actual == expected;
    }, 10000);
    if (!ok) {
        // actual.save("actual.png");
        // expected.save("expected.png");
        QFAIL("expected green square to be rendered");
    }
}

void tst_QQuickWebEngineViewGraphics::simpleGraphics()
{
    setHtml(greenSquare);
    verifyGreenSquare(m_view.data());
}

void tst_QQuickWebEngineViewGraphics::showHideShow()
{
    setHtml(greenSquare);
    QSignalSpy exposeSpy(m_view.data(), SIGNAL(exposeChanged()));
    m_view->show();
    QVERIFY(exposeSpy.wait());
    verifyGreenSquare(m_view.data());

    m_view->hide();
    QVERIFY(exposeSpy.wait());
    m_view->show();
    QVERIFY(exposeSpy.wait());
    verifyGreenSquare(m_view.data());
}

void tst_QQuickWebEngineViewGraphics::simpleAcceleratedLayer()
{
    setHtml(acLayerGreenSquare);
    verifyGreenSquare(m_view.data());
}

void tst_QQuickWebEngineViewGraphics::reparentToOtherWindow()
{
    setHtml(greenSquare);
    QQuickWindow window;
    window.resize(m_view->size());
    window.create();

    m_view->rootObject()->setParentItem(window.contentItem());
    verifyGreenSquare(&window);
}

void tst_QQuickWebEngineViewGraphics::setHtml(const QString &html)
{
    QString htmlData = QUrl::toPercentEncoding(html);
    QString qmlData = QUrl::toPercentEncoding(QStringLiteral("import QtQuick 2.0; import QtWebEngine 1.2; WebEngineView { width: 150; height: 150 }"));
    m_view->setSource(QUrl(QStringLiteral("data:text/plain,%1").arg(qmlData)));
    m_view->create();

    QQuickWebEngineView *webEngineView = static_cast<QQuickWebEngineView *>(m_view->rootObject());
    webEngineView->setProperty("url", QUrl(QStringLiteral("data:text/html,%1").arg(htmlData)));
    QTRY_COMPARE_WITH_TIMEOUT(m_view->rootObject()->property("loading"), QVariant(false), 30000);
}

static QByteArrayList params;
W_QTEST_MAIN(tst_QQuickWebEngineViewGraphics, params)
#include "tst_qquickwebengineviewgraphics.moc"
