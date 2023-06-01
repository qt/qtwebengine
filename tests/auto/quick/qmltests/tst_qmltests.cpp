// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <httpserver.h>

#if QT_CONFIG(ssl)
#include <httpsserver.h>
#endif

#include <QtCore/qscopedpointer.h>
#include <QtCore/qtemporarydir.h>
#include <QtGui/private/qinputmethod_p.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuickTest/quicktest.h>
#include <QtTest/qtest.h>
#include <QtWebEngineQuick/qquickwebengineprofile.h>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <qt_webengine_quicktest.h>

#if defined(Q_OS_LINUX) && defined(QT_DEBUG)
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif

class Setup : public QObject
{
    Q_OBJECT
public:
    Setup() { }

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        engine->addImportPath(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + "/mock-delegates");
    }
};

#if defined(Q_OS_LINUX) && defined(QT_DEBUG)
static bool debuggerPresent()
{
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd == -1)
      return false;
    char buffer[2048];
    ssize_t size = read(fd, buffer, sizeof(buffer) - 1);
    if (size == -1) {
      close(fd);
      return false;
    }
    buffer[size] = 0;
    const char tracerPidToken[] = "\nTracerPid:";
    char *tracerPid = strstr(buffer, tracerPidToken);
    if (!tracerPid) {
      close(fd);
      return false;
    }
    tracerPid += sizeof(tracerPidToken);
    long int pid = strtol(tracerPid, &tracerPid, 10);
    close(fd);
    return pid != 0;
}

static void stackTrace()
{
    bool ok = false;
    const int disableStackDump = qEnvironmentVariableIntValue("QTEST_DISABLE_STACK_DUMP", &ok);
    if (ok && disableStackDump == 1)
        return;

    if (debuggerPresent())
        return;

    fprintf(stderr, "\n========= Received signal, dumping stack ==============\n");
    char cmd[512];
    qsnprintf(cmd, 512, "gdb --pid %d 2>/dev/null <<EOF\n"
                        "set prompt\n"
                        "set height 0\n"
                        "thread apply all where full\n"
                        "detach\n"
                        "quit\n"
                        "EOF\n",
                        (int)getpid());

    if (system(cmd) == -1)
        fprintf(stderr, "calling gdb failed\n");
    fprintf(stderr, "========= End of stack trace ==============\n");
}

static void sigSegvHandler(int signum)
{
    stackTrace();
    qFatal("Received signal %d", signum);
}
#endif

class TempDir : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE QString path() {
        Q_ASSERT(tempDir.isValid());
        return tempDir.isValid() ? tempDir.path() : QString();
    }

    Q_INVOKABLE QUrl pathUrl(const QString &filename = QString())
    {
        Q_ASSERT(tempDir.isValid());
        return filename.isEmpty() ? QUrl::fromLocalFile(tempDir.path())
                                  : QUrl::fromLocalFile(tempDir.filePath(filename));
    }

    Q_INVOKABLE void removeRecursive(const QString dirname)
    {
        QDir dir(dirname);
        QFileInfoList entries(dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot));
        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i].isDir())
                removeRecursive(entries[i].filePath());
            else
                dir.remove(entries[i].fileName());
        }
        QDir().rmdir(dirname);
    }

    Q_INVOKABLE void createDirectory(const QString dirname) { QDir(tempDir.path()).mkdir(dirname); }

private:
    QTemporaryDir tempDir;
};

class TestInputContext : public QPlatformInputContext {
    Q_OBJECT

public:
    TestInputContext() = default;
    ~TestInputContext() { release(); }

    Q_INVOKABLE void create()
    {
        QInputMethodPrivate *inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = this;
    }

    Q_INVOKABLE void release()
    {
        QInputMethodPrivate *inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = nullptr;
    }

    void showInputPanel() override { m_visible = true; }
    void hideInputPanel() override { m_visible = false; }
    bool isInputPanelVisible() const override { return m_visible; }

private:
    bool m_visible = false;
};

QT_BEGIN_NAMESPACE
namespace QTest {
    int Q_TESTLIB_EXPORT defaultMouseDelay();
}
QT_END_NAMESPACE

class TestInputEvent : public QObject {
    Q_OBJECT

public:
    TestInputEvent() = default;

    Q_INVOKABLE bool mouseMultiClick(QObject *item, qreal x, qreal y, int clickCount)
    {
        QTEST_ASSERT(item);

        QWindow *view = eventWindow(item);
        if (!view)
            return false;

        for (int i = 0; i < clickCount; ++i) {
            mouseEvent(QMouseEvent::MouseButtonPress, view, item, QPointF(x, y));
            mouseEvent(QMouseEvent::MouseButtonRelease, view, item, QPointF(x, y));
        }
        QTest::lastMouseTimestamp += QTest::mouseDoubleClickInterval;

        return true;
    }

private:
    QWindow *eventWindow(QObject *item = nullptr)
    {
        QWindow *window = qobject_cast<QWindow *>(item);
        if (window)
            return window;

        QQuickItem *quickItem = qobject_cast<QQuickItem *>(item);
        if (quickItem)
            return quickItem->window();

        QQuickItem *testParentItem = qobject_cast<QQuickItem *>(parent());
        if (testParentItem)
            return testParentItem->window();

        return nullptr;
    }

    void mouseEvent(QEvent::Type type, QWindow *window, QObject *item, const QPointF &_pos)
    {
        QTest::qWait(QTest::defaultMouseDelay());
        QTest::lastMouseTimestamp += QTest::defaultMouseDelay();

        QPoint pos;
        QQuickItem *sgitem = qobject_cast<QQuickItem *>(item);
        if (sgitem)
            pos = sgitem->mapToScene(_pos).toPoint();

        QMouseEvent me(type, pos, window->mapFromGlobal(pos), Qt::LeftButton, Qt::LeftButton, {});
        me.setTimestamp(++QTest::lastMouseTimestamp);

        QSpontaneKeyEvent::setSpontaneous(&me);
        if (!qApp->notify(window, &me))
            qWarning("Mouse click event not accepted by receiving window");
    }
};

int main(int argc, char **argv)
{
#if defined(Q_OS_LINUX) && defined(QT_DEBUG)
    struct sigaction sigAction;

    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_handler = &sigSegvHandler;
    sigAction.sa_flags = 0;

    sigaction(SIGSEGV, &sigAction, 0);
#endif
    QtWebEngineQuick::initialize();
    // Force to use English language for testing due to error message checks
    QLocale::setDefault(QLocale("en"));

    static QByteArrayList params = {QByteArrayLiteral("--webEngineArgs"),QByteArrayLiteral("--use-fake-device-for-media-stream")};
    QList<const char *> w_argv(argc);
    for (int i = 0; i < argc; ++i) w_argv[i] = argv[i];
    for (int i = 0; i < params.size(); ++i) w_argv.append(params[i].data());
    int w_argc = w_argv.size();
    Application app(w_argc, const_cast<char **>(w_argv.data()));

    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    qmlRegisterType<TempDir>("Test.util", 1, 0, "TempDir");
    qmlRegisterType<TestInputContext>("Test.util", 1, 0, "TestInputContext");
    qmlRegisterType<TestInputEvent>("Test.util", 1, 0, "TestInputEvent");

    QTEST_SET_MAIN_SOURCE_PATH
    qmlRegisterSingletonType<HttpServer>("Test.Shared", 1, 0, "HttpServer", [&] (QQmlEngine *, QJSEngine *) {
        auto server = new HttpServer;
        server->setResourceDirs(
                { server->sharedDataDir(),
                  QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + QLatin1String("/data") });
        return server;
    });

#if QT_CONFIG(ssl)
    qmlRegisterSingletonType<HttpsServer>(
            "Test.Shared", 1, 0, "HttpsServer", [&](QQmlEngine *, QJSEngine *) {
                return new HttpsServer(":/resources/server.pem", ":/resources/server.key", "");
            });
#endif
    Setup setup;
    int i = quick_test_main_with_setup(
            argc, argv, "qmltests",
            qPrintable(QT_TESTCASE_BUILDDIR + QLatin1String("/webengine.qmltests")), &setup);
    return i;
}

#include "tst_qmltests.moc"
