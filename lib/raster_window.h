#ifndef QT_RASTER_WINDOW_H
#define QT_RASTER_WINDOW_H

#include <QWindow>
#include <QVBoxLayout>
#include <QWidget>

class BackingStoreQt;

namespace content {
	class RenderWidgetHostViewQt;
}

class RasterWindow : public QWidget
{
public:
    RasterWindow(content::RenderWidgetHostViewQt* view, QWidget *parent = 0);

    void setBackingStore(BackingStoreQt* backingStore);

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

class RasterWindowContainer : public QVBoxLayout
{
public:
	RasterWindowContainer()
		: m_currentRasterWindow(0)
	{ }

	~RasterWindowContainer()
	{
	}

public:
	void insert(RasterWindow* rasterWindow)
	{
		removeWidget(m_currentRasterWindow);
		addWidget(rasterWindow);
		m_currentRasterWindow = rasterWindow;
	}

private:
	RasterWindow* m_currentRasterWindow;
};

#endif
