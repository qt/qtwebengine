#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QBrush>
#include <QHash>
#include <QPixmap>
#include <QRunnable>

class QPdfDocument;

class PageCache : public QObject
{
    Q_OBJECT
public:
    PageCache(QPdfDocument *doc, qreal zoom);
    ~PageCache();

    QPixmap get(int page);

public slots:
    void run();

signals:
    void pagesNeeded();
    void pageReady(int page);

private:
    void insertPage(int page);

private:
    QThread *m_thread;
    QPdfDocument *m_doc;
    QHash<int, QPixmap> m_pageCache;
    qreal m_zoom;
    int m_lastPageRequested;
    QPixmap m_placeholderIcon;
    QBrush m_placeholderBackground;

    // performance statistics
    qreal m_minRenderTime;
    qreal m_maxRenderTime;
    qreal m_totalRenderTime;
    int m_totalPagesRendered;
};

#endif // PAGECACHE_H
