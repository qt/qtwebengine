#ifndef BLINQPAGE_H
#define BLINQPAGE_H

#include <QObject>
#include <QScopedPointer>

class QWindow;

class BlinqPagePrivate;

class Q_DECL_EXPORT BlinqPage
{
public:
    BlinqPage(int argc, char **argv);
    ~BlinqPage();

    QWindow *window();

private:
    QScopedPointer<BlinqPagePrivate> d;
};

#endif // BLINQPAGE_H
