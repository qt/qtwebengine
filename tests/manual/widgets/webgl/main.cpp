/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QOperatingSystemVersion>
#include <QtGui/QOpenGLContext>
#include <QtGui/QScreen>
#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWebEngineWidgets/QWebEngineView>

// Manual test for checking if WebGL (1 or 2) works.
// Set environment variable QTWEBENGINE_GL_TYPE to one of the following to try and switch
// the underlying GL implementation (mostly Windows): "desktop", "gles", "gles3", "software".

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    QSize sizeHint() const;

private:
    QWebEngineView *view = nullptr;
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget;

    QGroupBox *horizontalGroupBox = new QGroupBox;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *exButton1 = new QPushButton(QStringLiteral("Aquarium (WebGL 1)"));
    QPushButton *exButton2 = new QPushButton(QStringLiteral("Lots of objects (WebGL 1)"));
    QPushButton *exButton3 = new QPushButton(QStringLiteral("Instanced triangles (WebGL 2)"));

    buttonLayout->addWidget(exButton1);
    buttonLayout->addWidget(exButton2);
    buttonLayout->addWidget(exButton3);
    horizontalGroupBox->setLayout(buttonLayout);

    const QUrl exUrl1 =
            QUrl(QStringLiteral(
                     "http://webglsamples.org/aquarium/aquarium.html"));
    const QUrl exUrl2 =
            QUrl(QStringLiteral(
                     "http://webglsamples.org/lots-o-objects/lots-o-objects-draw-elements.html"));
    const QUrl exUrl3 =
            QUrl(QStringLiteral(
                     "http://webglsamples.org/WebGL2Samples/#transform_feedback_instanced"));

    view = new QWebEngineView;
    connect(exButton1, &QPushButton::clicked, view, [=](){
        view->setUrl(exUrl1);
    });
    connect(exButton2, &QPushButton::clicked, view, [=](){
        view->setUrl(exUrl2);
    });
    connect(exButton3, &QPushButton::clicked, view, [=](){
        view->setUrl(exUrl3);
    });

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(horizontalGroupBox);
    centralLayout->addWidget(view, 1);

    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    view->setUrl(QUrl(QStringLiteral("http://webglsamples.org/aquarium/aquarium.html")));
    setWindowTitle(tr("WebGL 1 and 2 examples"));
}

QSize MainWindow::sizeHint() const
{
    const QRect desktopRect = QGuiApplication::primaryScreen()->geometry();
    const QSize size = desktopRect.size() * qreal(0.9);
    return size;
}

bool isWindows()
{
    return QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows;
}

// Easy snippets to copy to command line for testing for UNIX only.
// QTWEBENGINE_GL_TYPE=desktop   ./webgl
// QTWEBENGINE_GL_TYPE=gles      ./webgl
// QTWEBENGINE_GL_TYPE=gles3     ./webgl
// QTWEBENGINE_GL_TYPE=software  ./webgl
int main(int argc, char *argv[])
{
    // Not all options are relevant for all platforms.
    const QString desktopGL = QStringLiteral("desktop");
    const QString angle = QStringLiteral("angle"); // Same as gles really, just an alias.
    const QString gles = QStringLiteral("gles"); // ANGLE on Windows.
    const QString gles3 = QStringLiteral("gles3"); // ANGLE on Windows.
    const QString softwareGL = QStringLiteral("software");

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QString glType = qEnvironmentVariable("QTWEBENGINE_GL_TYPE");
    if (glType.isEmpty()) {
        if (isWindows())
            glType = gles3;
        else
            glType = desktopGL;
    }

    if (glType == desktopGL) {
        QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
        qInfo() << QStringLiteral("Trying to use Desktop OpenGL.\n");
    } else if (glType == gles || glType == gles3 || glType == angle) {
        QApplication::setAttribute(Qt::AA_UseOpenGLES);
        if (glType == gles || glType == angle)
            qInfo() << QStringLiteral("Trying to use OpenGL ES 2.\n");
        if (glType == gles3)
            qInfo() << QStringLiteral("Trying to use OpenGL ES 3.\n");
    } else if (glType == softwareGL) {
        QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
        qInfo() << QStringLiteral("Trying to use software OpenGL.\n");
    }

    if (glType == gles3) {
        // Set OpenGL ES version 3.
        QSurfaceFormat format;
        format.setSamples(4);
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        format.setVersion(3, 0);
        QSurfaceFormat::setDefaultFormat(format);
    }

    QApplication a(argc, argv);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.scenegraph.general=true"));
    QOpenGLContext *globalSharedContext = QOpenGLContext::globalShareContext();
    qInfo() << "Global OpenGL context format: " << globalSharedContext->format() << "\n";

    MainWindow w;

    // Move middle-ish.
    const QRect desktopRect = QGuiApplication::primaryScreen()->geometry();
    const QSize pos = desktopRect.size() * qreal(0.1);
    w.move(pos.width(), pos.height());

    w.show();

    return a.exec();
}

#include "main.moc"
