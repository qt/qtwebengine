#include <QtGui>
#include <QtWidgets>

#include <blinqapplication.h>
#include <blinqpage.h>

int main(int argc, char **argv)
{
    printf("main called\n");
    BlinqApplication app(argc, argv);

    BlinqPage page;
//    page.window()->show();

    return app.exec();
}

