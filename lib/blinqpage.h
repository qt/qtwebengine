#ifndef BLINQPAGE_H
#define BLINQPAGE_H

#include <QObject>
#include <QScopedPointer>

class QWindow;

class BlinqPagePrivate;

class Q_DECL_EXPORT BlinqPage
{
public:
    BlinqPage();
    ~BlinqPage();

    QWindow *window();

private:
    QScopedPointer<BlinqPagePrivate> d;
};

#endif // BLINQPAGE_H
