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

public slots:
    void setDocument(QPdfDocument *doc);
    void setZoom(qreal factor);

private:
    void render(int page);

private:
    QPdfDocument *m_doc;
    QHash<int, QPixmap> m_pageCache;
    QBrush m_background;
    int m_pageSpacing;
    qreal m_zoom;
    qreal m_top; // 1.5 means start from the bottom half of page 1
    qreal m_screenResolution; // pixels per point
};

#endif // SEQUENTIALPAGEWIDGET_H
