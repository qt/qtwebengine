#ifndef NATIVE_VIEW_QT_H
#define NATIVE_VIEW_QT_H


#include <QWidget>
#include <QQuickPaintedItem>

class BackingStoreQt;
class QWindow;
class QQuickItem;
class QFocusEvent;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;

namespace content {
	class RenderWidgetHostViewQt;
}

class NativeViewQt {
public:
    virtual void setBackingStore(BackingStoreQt* backingStore) = 0;
    virtual QRectF screenRect() const = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isVisible() const = 0;
    virtual QWindow* window() const = 0;
    virtual void update() = 0;
};

class QWidgetNativeView : public QWidget, public NativeViewQt
{
public:
    QWidgetNativeView(content::RenderWidgetHostViewQt* view, QWidget *parent = 0);

    virtual void setBackingStore(BackingStoreQt* backingStore);
    virtual QRectF screenRect() const;
    virtual void show();
    virtual void hide();
    virtual bool isVisible() const;
    virtual QWindow* window() const;
    virtual void update();

    QPainter* painter();

protected:
	void paintEvent(QPaintEvent * event);
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *resizeEvent);

private:
    BackingStoreQt* m_backingStore;
    QPainter* m_painter;
    content::RenderWidgetHostViewQt *m_view;
};

class QQuickNativeView : public QQuickPaintedItem, public NativeViewQt
{
    Q_OBJECT
public:
    QQuickNativeView(content::RenderWidgetHostViewQt* view, QQuickItem *parent = 0);

    virtual void setBackingStore(BackingStoreQt* backingStore);
    virtual QRectF screenRect() const;
    virtual void show();
    virtual void hide();
    virtual bool isVisible() const;
    virtual QWindow* window() const;
    virtual void update();

    void paint(QPainter *painter);
    void resize(int width, int height);

    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent*);

protected Q_SLOTS:
    void resizeBackingStore();

protected:
    QSGNode* updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * data);

private:
    BackingStoreQt* m_backingStore;
    content::RenderWidgetHostViewQt *m_view;

};

#endif
