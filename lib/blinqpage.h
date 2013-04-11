#ifndef BLINQPAGE_H
#define BLINQPAGE_H

#include <QObject>
#include <QScopedPointer>

class QWindow;

class BlinqPagePrivate;

class BlinqPage
{
public:
    BlinqPage();
    ~BlinqPage();

    QWindow *window();

private:
    QScopedPointer<BlinqPagePrivate> d;
};

#endif // BLINQPAGE_H
