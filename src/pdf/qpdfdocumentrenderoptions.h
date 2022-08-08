// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
        None,
        Clockwise90,
        Clockwise180,
        Clockwise270
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
