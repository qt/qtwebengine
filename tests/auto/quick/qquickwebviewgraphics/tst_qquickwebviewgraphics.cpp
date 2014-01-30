/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickItem>
#include <QtQuick/private/qsgcontext_p.h>

class TestView : public QQuickView {
    Q_OBJECT
public:
    virtual void exposeEvent(QExposeEvent *e) Q_DECL_OVERRIDE {
        QQuickView::exposeEvent(e);
        emit exposeChanged();
    }

Q_SIGNALS:
    void exposeChanged();
};

class tst_QQuickWebViewGraphics : public QObject
{
    Q_OBJECT
public:
    tst_QQuickWebViewGraphics();
    virtual ~tst_QQuickWebViewGraphics();

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void simpleGraphics();
    void renderAfterNodeCleanup();
    void showHideShow();
    void simpleAcceleratedLayer();
    void reparentToOtherWindow();

private:
    void setHtml(const QString &html);
    QScopedPointer<TestView> m_view;
};

static const QString greenSquare("<div style=\"background-color: #00ff00; position:absolute; left:50px; top: 50px; width: 50px; height: 50px;\"></div>");
static const QString acLayerGreenSquare("<div style=\"background-color: #00ff00; position:absolute; left:50px; top: 50px; width: 50px; height: 50px; transform: translateZ(0); -webkit-transform: translateZ(0);\"></div>");

static QImage get150x150GreenReferenceImage()
{
    static QImage reference;
    if (reference.isNull()) {
        reference = QImage(150, 150, QImage::Format_RGB32);
        reference.fill(Qt::white);
        QPainter painter(&reference);
        painter.fillRect(50, 50, 50, 50, QColor("#00ff00"));
    }
    return reference;
}

tst_QQuickWebViewGraphics::tst_QQuickWebViewGraphics()
{
}

tst_QQuickWebViewGraphics::~tst_QQuickWebViewGraphics()
{
}

// This will be called before the first test function is executed.
// It is only called once.
void tst_QQuickWebViewGraphics::initTestCase()
{
#if defined(TST_QQUICKWEBVIEWGRAPHICS_SOFTWARE)
    qApp->setProperty("QQuickWebEngineView_DisableHardwareAcceleration", QVariant(true));
#else
    // This is currently needed by all QtWebEngine application using the HW accelerated QQuickWebView.
    // It enables sharing between the QOpenGLContext of all QQuickWindows of the application.
    // We have to do so until we expose a public API for it, or chose enable it by default in Qt 5.3.0.
    QOpenGLContext *shareContext = new QOpenGLContext;
    shareContext->create();
    QSGContext::setSharedOpenGLContext(shareContext);
#endif
}

void tst_QQuickWebViewGraphics::init()
{
    m_view.reset(new TestView);
}

void tst_QQuickWebViewGraphics::cleanup()
{
}

void tst_QQuickWebViewGraphics::simpleGraphics()
{
    setHtml(greenSquare);
    QCOMPARE(m_view->grabWindow(), get150x150GreenReferenceImage());
}

void tst_QQuickWebViewGraphics::renderAfterNodeCleanup()
{
    setHtml(greenSquare);

    // Do it twice in a row, if the window isn't visible, the scene graph is going to be trashed by QQuickWindow::grabWindow after the first render.
    QVERIFY(!m_view->isVisible());
    QCOMPARE(m_view->grabWindow(), get150x150GreenReferenceImage());
    QCOMPARE(m_view->grabWindow(), get150x150GreenReferenceImage());
}

void tst_QQuickWebViewGraphics::showHideShow()
{
    setHtml(greenSquare);
    QSignalSpy exposeSpy(m_view.data(), SIGNAL(exposeChanged()));
    m_view->show();
    QVERIFY(exposeSpy.wait(500));
    QCOMPARE(m_view->grabWindow(), get150x150GreenReferenceImage());

    m_view->hide();
    QVERIFY(exposeSpy.wait(500));
    m_view->show();
    QVERIFY(exposeSpy.wait(500));
    QCOMPARE(m_view->grabWindow(), get150x150GreenReferenceImage());
}

void tst_QQuickWebViewGraphics::simpleAcceleratedLayer()
{
    setHtml(acLayerGreenSquare);
    QCOMPARE(m_view->grabWindow(), get150x150GreenReferenceImage());
}

void tst_QQuickWebViewGraphics::reparentToOtherWindow()
{
    setHtml(greenSquare);
    QQuickWindow window;
    window.resize(m_view->size());
    window.create();

    m_view->rootObject()->setParentItem(window.contentItem());
    QCOMPARE(window.grabWindow(), get150x150GreenReferenceImage());
}

void tst_QQuickWebViewGraphics::setHtml(const QString &html)
{
    QString htmlData = QUrl::toPercentEncoding(html);
    QString qmlData = QUrl::toPercentEncoding(QStringLiteral("import QtQuick 2.0; import QtWebEngine 1.0; WebEngineView { width: 150; height: 150; url: loadUrl }"));
    m_view->rootContext()->setContextProperty("loadUrl", QUrl(QStringLiteral("data:text/html,%1").arg(htmlData)));
    m_view->setSource(QUrl(QStringLiteral("data:text/plain,%1").arg(qmlData)));
    m_view->create();

    // FIXME: Replace with a proper initialLayout signal.
    QTest::qWait(200);
    QCOMPARE(m_view->rootObject()->property("loading"), QVariant(false));
}

QTEST_MAIN(tst_QQuickWebViewGraphics)
#include "tst_qquickwebviewgraphics.moc"
