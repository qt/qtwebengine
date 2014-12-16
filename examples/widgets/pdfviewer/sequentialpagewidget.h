#ifndef SEQUENTIALPAGEWIDGET_H
#define SEQUENTIALPAGEWIDGET_H

#include <QWidget>

class QPdfDocument;
class PageRenderer;

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
    void openDocument(const QUrl &url);
    void setZoom(qreal factor);
    void invalidate();

signals:
    void showingPageRange(int start, int end);
    void zoomChanged(qreal factor);

private slots:
    void pageLoaded(int page, qreal zoom, QImage image);

private:
    int pageCount();
    QSizeF pageSize(int page);
    void render(int page);

private:
    QHash<int, QImage> m_pageCache;
    QList<int> m_cachedPagesLRU;
    int m_pageCacheLimit;
    QVector<QSizeF> m_pageSizes;
    PageRenderer *m_pageRenderer;
    QBrush m_background;
    QPixmap m_placeholderIcon;
    QBrush m_placeholderBackground;
    int m_pageSpacing;
    int m_topPageShowing;
    int m_bottomPageShowing;
    QSize m_totalSize;
    qreal m_zoom;
    qreal m_screenResolution; // pixels per point
};

#endif // SEQUENTIALPAGEWIDGET_H
