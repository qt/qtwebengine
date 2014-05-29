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

#ifndef STREAM_VIDEO_NODE_H
#define STREAM_VIDEO_NODE_H

#include <QtQuick/qsgmaterial.h>
#include <QtQuick/qsgnode.h>

QT_BEGIN_NAMESPACE
class QSGTexture;
QT_END_NAMESPACE

// These classes duplicate, QtQuick style, the logic of GLRenderer::DrawStreamVideoQuad.
// Their behavior should stay as close as possible to GLRenderer.

class StreamVideoMaterial : public QSGMaterial
{
public:
    StreamVideoMaterial(QSGTexture *texture);

    virtual QSGMaterialType *type() const Q_DECL_OVERRIDE{
        static QSGMaterialType theType;
        return &theType;
    }

    virtual QSGMaterialShader *createShader() const;

    QSGTexture *m_texture;
    QMatrix4x4 m_texMatrix;
};

class StreamVideoNode : public QSGGeometryNode
{
public:
    StreamVideoNode(QSGTexture *texture);
    void setRect(const QRectF &rect);
    void setTextureMatrix(const QMatrix4x4 &matrix);

private:
    QSGGeometry m_geometry;
    StreamVideoMaterial *m_material;
};

#endif // STREAM_VIDEO_NODE_H
