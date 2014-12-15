#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QBrush>
#include <QHash>
#include <QPixmap>
#include <QRunnable>
#include <QThread>
#include <QPdfDocument>

class QPdfDocument;

class PageRenderer : public QThread
{
    Q_OBJECT
public:
    PageRenderer();
    ~PageRenderer();

public slots:
    QVector<QSizeF> openDocument(const QUrl &location);
    void requestPage(int page, qreal zoom, Priority priority = QThread::NormalPriority);

signals:
    void pageReady(int page, qreal zoom, QImage image);

protected:
    Q_DECL_OVERRIDE void run();

private:
    void renderPage(int page, qreal zoom);

private:
    QPdfDocument m_doc;

    // current request only
    int m_page;
    qreal m_zoom;

    // performance statistics
    qreal m_minRenderTime;
    qreal m_maxRenderTime;
    qreal m_totalRenderTime;
    int m_totalPagesRendered;
};

#endif // PAGECACHE_H
