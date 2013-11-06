/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
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

#ifndef YUV_VIDEO_NODE_H
#define YUV_VIDEO_NODE_H

#include <QtQuick/qsgmaterial.h>
#include <QtQuick/qsgnode.h>

class QSGTexture;

// These classes duplicate, QtQuick style, the logic of GLRenderer::DrawYUVVideoQuad.
// Their behavior should stay as close as possible to GLRenderer.

class YUVVideoMaterial : public QSGMaterial
{
public:
    YUVVideoMaterial(QSGTexture *yTexture, QSGTexture *uTexture, QSGTexture *vTexture, const QSizeF &texScale);

    virtual QSGMaterialType *type() const {
        static QSGMaterialType theType;
        return &theType;
    }

    virtual QSGMaterialShader *createShader() const;
    virtual int compare(const QSGMaterial *other) const;

    QSGTexture *m_yTexture;
    QSGTexture *m_uTexture;
    QSGTexture *m_vTexture;
    QSizeF m_texScale;
};

class YUVAVideoMaterial : public YUVVideoMaterial
{
public:
    YUVAVideoMaterial(QSGTexture *yTexture, QSGTexture *uTexture, QSGTexture *vTexture, QSGTexture *aTexture, const QSizeF &texScale);

    virtual QSGMaterialType *type() const {
        static QSGMaterialType theType;
        return &theType;
    }

    virtual QSGMaterialShader *createShader() const;
    virtual int compare(const QSGMaterial *other) const;

    QSGTexture *m_aTexture;
};


class YUVVideoNode : public QSGGeometryNode
{
public:
    YUVVideoNode(QSGTexture *yTexture, QSGTexture *uTexture, QSGTexture *vTexture, QSGTexture *aTexture, const QSizeF &texScale);
    void setRect(const QRectF &rect);

private:
    QSGGeometry m_geometry;
    YUVVideoMaterial *m_material;
};

#endif // YUV_VIDEO_NODE_H
