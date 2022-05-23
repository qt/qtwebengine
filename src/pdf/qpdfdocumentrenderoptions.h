/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QPDFDOCUMENTRENDEROPTIONS_H
#define QPDFDOCUMENTRENDEROPTIONS_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

class QPdfDocumentRenderOptions
{
public:
    enum class Rotation {
        Rotate0,
        Rotate90,
        Rotate180,
        Rotate270
    };

    enum class RenderFlag {
        None = 0x000,
        Annotations = 0x001,
        OptimizedForLcd = 0x002,
        Grayscale = 0x004,
        ForceHalftone = 0x008,
        TextAliased = 0x010,
        ImageAliased = 0x020,
        PathAliased = 0x040
    };
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    constexpr QPdfDocumentRenderOptions() noexcept : m_renderFlags(0), m_rotation(0), m_reserved(0) {}

    constexpr Rotation rotation() const noexcept { return static_cast<Rotation>(m_rotation); }
    constexpr void setRotation(Rotation r) noexcept { m_rotation = quint32(r); }

    constexpr RenderFlags renderFlags() const noexcept { return static_cast<RenderFlags>(m_renderFlags); }
    constexpr void setRenderFlags(RenderFlags r) noexcept { m_renderFlags = quint32(r.toInt()); }

    constexpr QRect scaledClipRect() const noexcept { return m_clipRect; }
    constexpr void setScaledClipRect(const QRect &r) noexcept { m_clipRect = r; }

    constexpr QSize scaledSize() const noexcept { return m_scaledSize; }
    constexpr void setScaledSize(const QSize &s) noexcept { m_scaledSize = s; }

private:
    friend constexpr inline bool operator==(const QPdfDocumentRenderOptions &lhs, const QPdfDocumentRenderOptions &rhs) noexcept;

    QRect m_clipRect;
    QSize m_scaledSize;

    quint32 m_renderFlags : 8;
    quint32 m_rotation    : 3;
    quint32 m_reserved    : 21;
    quint32 m_reserved2 = 0;
};

Q_DECLARE_TYPEINFO(QPdfDocumentRenderOptions, Q_PRIMITIVE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(QPdfDocumentRenderOptions::RenderFlags)

constexpr inline bool operator==(const QPdfDocumentRenderOptions &lhs, const QPdfDocumentRenderOptions &rhs) noexcept
{
    return lhs.m_clipRect == rhs.m_clipRect && lhs.m_scaledSize == rhs.m_scaledSize &&
            lhs.m_renderFlags == rhs.m_renderFlags && lhs.m_rotation == rhs.m_rotation &&
            lhs.m_reserved == rhs.m_reserved && lhs.m_reserved2 == rhs.m_reserved2; // fix -Wunused-private-field
}

constexpr inline bool operator!=(const QPdfDocumentRenderOptions &lhs, const QPdfDocumentRenderOptions &rhs) noexcept
{
    return !operator==(lhs, rhs);
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPdfDocumentRenderOptions)

#endif // QPDFDOCUMENTRENDEROPTIONS_H
