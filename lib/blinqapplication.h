#ifndef BLINQAPPLICATION_H
#define BLINQAPPLICATION_H

#include <QApplication>

class Q_DECL_EXPORT BlinqApplication : public QApplication {
public:
    BlinqApplication(int &argc, char **argv);
	static int exec();
};

#endif