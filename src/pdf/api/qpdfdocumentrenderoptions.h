/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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

#ifndef QPDFDOCUMENTRENDEROPTIONS_H
#define QPDFDOCUMENTRENDEROPTIONS_H

#include <QtPdf/qpdfnamespace.h>
#include <QtCore/qobject.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

class QPdfDocumentRenderOptions
{
public:
    Q_DECL_CONSTEXPR QPdfDocumentRenderOptions() noexcept : m_renderFlags(0), m_rotation(0), m_reserved(0) {}

    Q_DECL_CONSTEXPR QPdf::Rotation rotation() const noexcept { return static_cast<QPdf::Rotation>(m_rotation); }
    Q_DECL_RELAXED_CONSTEXPR void setRotation(QPdf::Rotation r) noexcept { m_rotation = r; }

    Q_DECL_CONSTEXPR QPdf::RenderFlags renderFlags() const noexcept { return static_cast<QPdf::RenderFlags>(m_renderFlags); }
    Q_DECL_RELAXED_CONSTEXPR void setRenderFlags(QPdf::RenderFlags r) noexcept { m_renderFlags = r; }

    Q_DECL_CONSTEXPR QRect scaledClipRect() const noexcept { return m_clipRect; }
    Q_DECL_RELAXED_CONSTEXPR void setScaledClipRect(const QRect &r) noexcept { m_clipRect = r; }

    Q_DECL_CONSTEXPR QSize scaledSize() const noexcept { return m_scaledSize; }
    Q_DECL_RELAXED_CONSTEXPR void setScaledSize(const QSize &s) noexcept { m_scaledSize = s; }

private:
    friend Q_DECL_CONSTEXPR inline bool operator==(QPdfDocumentRenderOptions lhs, QPdfDocumentRenderOptions rhs) noexcept;

    QRect m_clipRect;
    QSize m_scaledSize;

    quint32 m_renderFlags : 8;
    quint32 m_rotation    : 3;
    quint32 m_reserved    : 21;
    quint32 m_reserved2 = 0;
};

Q_DECLARE_TYPEINFO(QPdfDocumentRenderOptions, Q_PRIMITIVE_TYPE);

Q_DECL_CONSTEXPR inline bool operator==(QPdfDocumentRenderOptions lhs, QPdfDocumentRenderOptions rhs) noexcept
{
    return lhs.m_clipRect == rhs.m_clipRect && lhs.m_scaledSize == rhs.m_scaledSize &&
            lhs.m_renderFlags == rhs.m_renderFlags && lhs.m_rotation == rhs.m_rotation &&
            lhs.m_reserved == rhs.m_reserved && lhs.m_reserved2 == rhs.m_reserved2; // fix -Wunused-private-field
}

Q_DECL_CONSTEXPR inline bool operator!=(QPdfDocumentRenderOptions lhs, QPdfDocumentRenderOptions rhs) noexcept
{
    return !operator==(lhs, rhs);
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPdfDocumentRenderOptions)

#endif // QPDFDOCUMENTRENDEROPTIONS_H
