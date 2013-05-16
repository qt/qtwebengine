#ifndef QT_RASTER_WINDOW_H
#define QT_RASTER_WINDOW_H

#include <QWindow>
#include <QVBoxLayout>
#include <QWidget>

class BackingStoreQt;

namespace content {
	class RenderWidgetHostViewQt;
}

class RasterWindow : public QWindow
{
public:
    RasterWindow(content::RenderWidgetHostViewQt* view, QWindow *parent = 0);

    void renderNow();
    void setBackingStore(BackingStoreQt* backingStore);

protected:

    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *resizeEvent);
    void exposeEvent(QExposeEvent *);

private:
    BackingStoreQt* m_backingStore;
    content::RenderWidgetHostViewQt *m_view;

};

class RasterWindowContainer : public QVBoxLayout
{
public:
	RasterWindowContainer()
		: m_windowContainer(0)
		, m_currentRasterWindow(0)
	{ }

	~RasterWindowContainer()
	{
		if (m_windowContainer)
			delete m_windowContainer;
	}

public:
	void insert(RasterWindow* rasterWindow)
	{
		if (m_windowContainer) {
			removeWidget(m_windowContainer);

			// Before deleting m_windowContainer we have to hide and reparent the contained RasterWindow.
			// The RasterWindow is in fact being owned and being deleted by the RenderWidgetHostView.
			m_currentRasterWindow->hide();
			m_currentRasterWindow->setParent(0);

			delete m_windowContainer;
			m_windowContainer = 0;
		}

		m_windowContainer = QWidget::createWindowContainer(rasterWindow);
		addWidget(m_windowContainer);

		m_currentRasterWindow = rasterWindow;
	}

private:
	QWidget* m_windowContainer;
	RasterWindow* m_currentRasterWindow;
};

#endif
