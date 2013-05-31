/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
