#ifndef SEQUENTIALPAGEWIDGET_H
#define SEQUENTIALPAGEWIDGET_H

#include <QWidget>
#include <QDebug>

class QPdfDocument;

class SequentialPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequentialPageWidget(QWidget *parent = 0);
    ~SequentialPageWidget();

    void paintEvent(QPaintEvent * event);

public slots:
    void setDocument(QPdfDocument *doc);
    void setZoom(qreal factor);
    void invalidate();

signals:
    void showingPageRange(int start, int end);

private:
    QSizeF pageSize(int page);
    void render(int page);

private:
    QPdfDocument *m_doc;
    QHash<int, QPixmap> m_pageCache;
    QBrush m_background;
    int m_pageSpacing;
    QSize m_totalSize;
    qreal m_zoom;
    qreal m_screenResolution; // pixels per point
};

#endif // SEQUENTIALPAGEWIDGET_H
