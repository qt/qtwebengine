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

#ifndef CONTENT_BROWSER_RENDERER_HOST_BACKING_STORE_QT_H_
#define CONTENT_BROWSER_RENDERER_HOST_BACKING_STORE_QT_H_

#include "content/browser/renderer_host/backing_store.h"

#include <QPainter>
#include <QPixmap>

class BackingStoreQt : public content::BackingStore
{
public:
    BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size, QWindow* parent);
    ~BackingStoreQt();

    void resize(const QSize& size);
    void paintToTarget(QPainter*, const QRect& rect);

    virtual void PaintToBackingStore(content::RenderProcessHost *process, TransportDIB::Id bitmap, const gfx::Rect &bitmap_rect,
                                     const std::vector<gfx::Rect> &copy_rects, float scale_factor, const base::Closure &completion_callback,
                                     bool *scheduled_completion_callback);

    virtual void ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size);
    virtual bool CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output);

private:
    QPainter m_painter;
    content::RenderWidgetHost* m_host;
    QPixmap m_pixelBuffer;
    bool m_isValid;
};

#endif
