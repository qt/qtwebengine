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

#include "backing_store_qt.h"

#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/render_process_host.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"

#include <QPainter>

BackingStoreQt::BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size, QWindow* parent)
    : m_host(content::RenderWidgetHostImpl::From(host))
    , m_pixelBuffer(size.width(), size.height())
    , content::BackingStore(host, size)
    , m_isValid(false)
{
    int width = size.width();
    int height = size.height();
    resize(QSize(width, height));
}

BackingStoreQt::~BackingStoreQt()
{
}

void BackingStoreQt::resize(const QSize& size)
{
    m_isValid = false;
    if (size != m_pixelBuffer.size()) {
        QPixmap oldBackingStore = m_pixelBuffer;
        m_pixelBuffer = QPixmap(size);

        QPainter painter(&m_pixelBuffer);
        painter.drawPixmap(oldBackingStore.rect(), oldBackingStore);

        m_host->WasResized();
    }
}

void BackingStoreQt::paintToTarget(QPainter* painter, const QRectF& rect)
{
    if (m_pixelBuffer.isNull())
        return;
    painter->drawPixmap(rect, m_pixelBuffer, rect);
}

void BackingStoreQt::PaintToBackingStore(content::RenderProcessHost *process,
                                 TransportDIB::Id bitmap,
                                 const gfx::Rect &bitmap_rect,
                                 const std::vector<gfx::Rect> &copy_rects,
                                 float scale_factor,
                                 const base::Closure &completion_callback,
                                 bool *scheduled_completion_callback)
{
    if (bitmap_rect.IsEmpty())
        return;

    *scheduled_completion_callback = false;
    TransportDIB* dib = process->GetTransportDIB(bitmap);
    if (!dib)
      return;

    gfx::Rect pixel_bitmap_rect = bitmap_rect;

    uint8_t* bitmapData = static_cast<uint8_t*>(dib->memory());
    int width = m_pixelBuffer.size().width();
    int height = m_pixelBuffer.size().height();
    QImage img(bitmapData, pixel_bitmap_rect.width(), pixel_bitmap_rect.height(), QImage::Format_ARGB32);

    m_painter.begin(&m_pixelBuffer);

    for (size_t i = 0; i < copy_rects.size(); ++i) {
        gfx::Rect copy_rect = gfx::ToEnclosedRect(gfx::ScaleRect(copy_rects[i], scale_factor));

        QRect source = QRect( copy_rect.x() - pixel_bitmap_rect.x()
                            , copy_rect.y() - pixel_bitmap_rect.y()
                            , copy_rect.width()
                            , copy_rect.height());

        QRect destination = QRect( copy_rect.x()
                                 , copy_rect.y()
                                 , copy_rect.width()
                                 , copy_rect.height());

        m_painter.drawPixmap(destination, QPixmap::fromImage(img), source);
    }

    m_painter.end();
}

void BackingStoreQt::ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size)
{
    DCHECK(delta.x() == 0 || delta.y() == 0);

    m_pixelBuffer.scroll(delta.x(), delta.y(), clip_rect.x(), clip_rect.y(), clip_rect.width(), clip_rect.height());
}

bool BackingStoreQt::CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output)
{
    // const int width = std::min(m_pixelBuffer.width(), rect.width());
    // const int height = std::min(m_pixelBuffer.height(), rect.height());

    // if (!output->Allocate(width, height, true))
    //     return false;

    // // This code assumes a visual mode where a pixel is
    // // represented using a 32-bit unsigned int, with a byte per component.
    // const SkBitmap& bitmap = output->GetBitmap();
    // SkAutoLockPixels alp(bitmap);

    // QPixmap cpy = m_pixelBuffer.copy(rect.x(), rect.y(), rect.width(), rect.height());
    // QImage img = cpy.toImage();

    // // Convert the format and remove transparency.
    // if (img.format() != QImage::Format_RGB32)
    //     img = img.convertToFormat(QImage::Format_RGB32);

    // const uint8_t* src = img.bits();
    // uint8_t* dst = reinterpret_cast<uint8_t*>(bitmap.getAddr32(0,0));
    // memcpy(dst, src, width*height*32);

    // return true;
}

