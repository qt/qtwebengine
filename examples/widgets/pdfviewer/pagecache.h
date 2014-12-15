#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QBrush>
#include <QHash>
#include <QPixmap>
#include <QRunnable>
#include <QThread>

class QPdfDocument;

class PageCache : public QThread
{
    Q_OBJECT
public:
    PageCache(QPdfDocument *doc, qreal zoom);
    ~PageCache();

    QPixmap get(int page);

signals:
    void pageReady(int page);

protected:
    Q_DECL_OVERRIDE void run();

private:
    void insertPage(int page);

private:
    QPdfDocument *m_doc;
    QHash<int, QPixmap> m_pageCache;
    qreal m_zoom;
    int m_lastPageRequested;

    // performance statistics
    qreal m_minRenderTime;
    qreal m_maxRenderTime;
    qreal m_totalRenderTime;
    int m_totalPagesRendered;
};

#endif // PAGECACHE_H
