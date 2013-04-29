#include <QtGui>

#include <blinqpage.h>

int main(int argc, char **argv)
{
    printf("main called\n");
    QGuiApplication app(argc, argv);

    BlinqPage page(argc, argv);
//    page.window()->show();

    return app.exec();
}

