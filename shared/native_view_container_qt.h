#ifndef NATIVE_VIEW_CONTAINER_QT_H
#define NATIVE_VIEW_CONTAINER_QT_H

#include "native_view_qt.h"

#include <QWindow>
#include <QVBoxLayout>
#include <QQuickItem>

class NativeViewContainerQt : public QObject
{
    Q_OBJECT
public:
    NativeViewContainerQt()
        : m_embeddable(0)
        , m_currentQQuickNativeView(0)
        , m_currentQWidgetNativeView(0)
        , m_isQQuick(false)
    {
    }

    QQuickItem* qQuickItem()
    {
        if (!m_embeddable) {
            QQuickItem* embeddable = new QQuickItem;
            m_isQQuick = true;
            connect(embeddable, SIGNAL(widthChanged()), this, SLOT(resized()));
            connect(embeddable, SIGNAL(heightChanged()), this, SLOT(resized()));
            m_embeddable = embeddable;
        }

        return static_cast<QQuickItem*>(m_embeddable);
    }

    QVBoxLayout* widget()
    {
        if (!m_embeddable)
            m_embeddable = new QVBoxLayout;
        return static_cast<QVBoxLayout*>(m_embeddable);
    }

    void setWidth(qreal width)
    {
        if (m_isQQuick && m_currentQQuickNativeView) {
            m_currentQQuickNativeView->setWidth(width);
            m_currentQQuickNativeView->setContentsSize(QSize(width, m_currentQQuickNativeView->height()));
            qQuickItem()->setWidth(width);
        }
    }

    void setHeight(qreal height)
    {
        if (m_isQQuick && m_currentQQuickNativeView) {
            m_currentQQuickNativeView->setHeight(height);
            m_currentQQuickNativeView->setContentsSize(QSize(m_currentQQuickNativeView->width(), height));
            qQuickItem()->setHeight(height);
        }
    }

    NativeViewQt* createNativeView(content::RenderWidgetHostViewQt* renderWidgetHostView)
    {
        if (m_isQQuick) {
            insert(new QQuickNativeView(renderWidgetHostView));
            return m_currentQQuickNativeView;
        }

        insert(new QWidgetNativeView(renderWidgetHostView));
        return m_currentQWidgetNativeView;
    }

protected:
    void insert(QWidgetNativeView* nativeView)
    {
        widget()->removeWidget(m_currentQWidgetNativeView);
        widget()->addWidget(nativeView);
        m_currentQWidgetNativeView = nativeView;
    }

    void insert(QQuickNativeView* nativeView)
    {
        fprintf(stderr, "replace: %p with %p\n", m_currentQQuickNativeView, nativeView);
        if (m_currentQQuickNativeView)
            m_currentQQuickNativeView->setParentItem(0);

        nativeView->setParentItem(qQuickItem());
        m_currentQQuickNativeView = nativeView;
        setWidth(qQuickItem()->width());
        setHeight(qQuickItem()->height());
    }

public Q_SLOTS:
    void resized()
    {
        int w = static_cast<unsigned int>(qQuickItem()->width());
        int h = static_cast<unsigned int>(qQuickItem()->height());
        if (m_currentQQuickNativeView)
            m_currentQQuickNativeView->resize(w, h);
    }

private:
    QObject* m_embeddable;
    QWidgetNativeView* m_currentQWidgetNativeView;
    QQuickNativeView* m_currentQQuickNativeView;
    bool m_isQQuick;
};

#endif
