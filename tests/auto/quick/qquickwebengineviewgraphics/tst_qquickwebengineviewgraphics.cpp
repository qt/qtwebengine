// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <quickutil.h>
#include <QtTest/QtTest>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickItem>
#include <QPainter>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>

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
        if (actual.height() > 150)
            actual = actual.scaledToHeight(150);
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
    m_view->show();
    verifyGreenSquare(m_view.data());
    m_view->hide();
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
    m_view->hide();
}

void tst_QQuickWebEngineViewGraphics::simpleAcceleratedLayer()
{
    m_view->show();
    setHtml(acLayerGreenSquare);
    verifyGreenSquare(m_view.data());
    m_view->hide();
}

void tst_QQuickWebEngineViewGraphics::reparentToOtherWindow()
{
    setHtml(greenSquare);
    QQuickWindow window;
    window.resize(m_view->size());
    window.create();

    m_view->rootObject()->setParentItem(window.contentItem());
    window.show();
    verifyGreenSquare(&window);
}

void tst_QQuickWebEngineViewGraphics::setHtml(const QString &html)
{
    QString htmlData = QUrl::toPercentEncoding(html);
    QString qmlData = QUrl::toPercentEncoding(QStringLiteral("import QtQuick; import QtWebEngine; WebEngineView { width: 150; height: 150 }"));
    m_view->setSource(QUrl(QStringLiteral("data:text/plain,%1").arg(qmlData)));
    m_view->create();

    QQuickWebEngineView *webEngineView = static_cast<QQuickWebEngineView *>(m_view->rootObject());
    webEngineView->setUrl(QUrl(QStringLiteral("data:text/html,%1").arg(htmlData)));
    QVERIFY(waitForLoadSucceeded(webEngineView));
}

static QByteArrayList params;
W_QTEST_MAIN(tst_QQuickWebEngineViewGraphics, params)
#include "tst_qquickwebengineviewgraphics.moc"
#include "moc_quickutil.cpp"
