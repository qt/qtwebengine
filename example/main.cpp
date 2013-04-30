#include <QtGui>
#include <QtWidgets>

#include <blinqpage.h>

int main(int argc, char **argv)
{
    printf("main called\n");
    QApplication app(argc, argv);

    BlinqPage page(argc, argv);
//    page.window()->show();

    return app.exec();
}

