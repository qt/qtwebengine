#ifndef SEQUENTIALPAGEWIDGET_H
#define SEQUENTIALPAGEWIDGET_H

#include <QWidget>

class QPdfDocument;

class SequentialPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequentialPageWidget(QWidget *parent = 0);
    ~SequentialPageWidget();

    void paintEvent(QPaintEvent * event);
    qreal zoom() { return m_zoom; }
    qreal yForPage(int page);
    int topPageShowing() { return m_topPageShowing; }
    int bottomPageShowing() { return m_bottomPageShowing; }

public slots:
    void setDocument(QPdfDocument *doc);
    void setZoom(qreal factor);
    void invalidate();

signals:
    void showingPageRange(int start, int end);
    void zoomChanged(qreal factor);

private:
    QSizeF pageSize(int page);
    void render(int page);

private:
    QPdfDocument *m_doc;
    QHash<int, QPixmap> m_pageCache;
    QBrush m_background;
    int m_pageSpacing;
    int m_topPageShowing;
    int m_bottomPageShowing;
    QSize m_totalSize;
    qreal m_zoom;
    qreal m_screenResolution; // pixels per point

    // performance statistics
    qreal m_minRenderTime;
    qreal m_maxRenderTime;
    qreal m_totalRenderTime;
    int m_totalPagesRendered;
};

#endif // SEQUENTIALPAGEWIDGET_H
