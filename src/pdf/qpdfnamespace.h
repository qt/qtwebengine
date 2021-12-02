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

#ifndef QPDFNAMESPACE_H
#define QPDFNAMESPACE_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

namespace QPdf {
    Q_NAMESPACE

    enum Rotation {
        Rotate0,
        Rotate90,
        Rotate180,
        Rotate270
    };
    Q_ENUM_NS(Rotation)

    enum RenderFlag {
        NoRenderFlags = 0x000,
        RenderAnnotations = 0x001,
        RenderOptimizedForLcd = 0x002,
        RenderGrayscale = 0x004,
        RenderForceHalftone = 0x008,
        RenderTextAliased = 0x010,
        RenderImageAliased = 0x020,
        RenderPathAliased = 0x040
    };
    Q_FLAG_NS(RenderFlag)
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)
    Q_DECLARE_OPERATORS_FOR_FLAGS(RenderFlags)
}

QT_END_NAMESPACE
#endif
