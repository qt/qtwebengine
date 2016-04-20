/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "type_conversion.h"

namespace QtWebEngineCore {

QImage toQImage(const SkBitmap &bitmap)
{
    QImage image;
    switch (bitmap.colorType()) {
    case kUnknown_SkColorType:
        break;
    case kAlpha_8_SkColorType:
        image = toQImage(bitmap, QImage::Format_Alpha8);
        break;
    case kRGB_565_SkColorType:
        image = toQImage(bitmap, QImage::Format_RGB16);
        break;
    case kARGB_4444_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kUnpremul_SkAlphaType:
            // not supported - treat as opaque
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGB444);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_ARGB4444_Premultiplied);
            break;
        }
        break;
    case kRGBA_8888_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBX8888);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBA8888_Premultiplied);
            break;
        case kUnpremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBA8888);
            break;
        }
        break;
    case kBGRA_8888_SkColorType:
        // we are assuming little-endian arch here.
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGB32);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_ARGB32_Premultiplied);
            break;
        case kUnpremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_ARGB32);
            break;
        }
        break;
    case kIndex_8_SkColorType: {
        image = toQImage(bitmap, QImage::Format_Indexed8);
        SkColorTable *skTable = bitmap.getColorTable();
        if (skTable) {
            QVector<QRgb> qTable(skTable->count());
            for (int i = 0; i < skTable->count(); ++i)
                qTable[i] = (*skTable)[i];
            image.setColorTable(qTable);
        }
        break;
    }
    case kGray_8_SkColorType:
        image = toQImage(bitmap, QImage::Format_Grayscale8);
        break;
    }
    return image;
}

} // namespace QtWebEngineCore
