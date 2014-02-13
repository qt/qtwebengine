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
#include "stream_video_node.h"

#include <QtQuick/qsgtexture.h>

class StreamVideoMaterialShader : public QSGMaterialShader
{
public:
    virtual void updateState(const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial);

    virtual char const *const *attributeNames() const {
        static const char *names[] = {
            "a_position",
            "a_texCoord",
            0
        };
        return names;
    }

protected:
    virtual const char *vertexShader() const {
        // Keep in sync with cc::VertexShaderVideoTransform
        const char *shader =
        "attribute highp vec4 a_position;\n"
        "attribute mediump vec2 a_texCoord;\n"
        "uniform highp mat4 matrix;\n"
        "varying mediump vec2 v_texCoord;\n"
        "void main() {\n"
        "  gl_Position = matrix * a_position;\n"
        "  v_texCoord = a_texCoord;\n"
        "}";
        return shader;
    }

    virtual const char *fragmentShader() const {
        // Keep in sync with cc::FragmentShaderOESImageExternal
        static const char *shader =
        "#extension GL_OES_EGL_image_external : require\n"
        "varying mediump vec2 v_texCoord;\n"
        "uniform samplerExternalOES s_texture;\n"
        "void main() {\n"
        "  vec4 texColor = texture2D(s_texture, v_texCoord);\n"
        "  gl_FragColor = texColor;\n"
        "}";
        return shader;
    }

    virtual void initialize() {
        m_id_matrix = program()->uniformLocation("matrix");
        m_id_sTexture = program()->uniformLocation("s_texture");
    }

    int m_id_matrix;
    int m_id_sTexture;
};

void StreamVideoMaterialShader::updateState(const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    Q_UNUSED(oldMaterial);

    StreamVideoMaterial *mat = static_cast<StreamVideoMaterial *>(newMaterial);
    program()->setUniformValue(m_id_sTexture, 0);

    mat->m_texture->bind();

    // TODO(mchishtie): handle state.opacity() when shader implements it
    if (state.isMatrixDirty())
        program()->setUniformValue(m_id_matrix, state.combinedMatrix());
}

StreamVideoMaterial::StreamVideoMaterial(QSGTexture *texture)
    : m_texture(texture)
{
}

QSGMaterialShader *StreamVideoMaterial::createShader() const
{
    return new StreamVideoMaterialShader;
}

int StreamVideoMaterial::compare(const QSGMaterial *other) const
{
    const StreamVideoMaterial *m = static_cast<const StreamVideoMaterial *>(other);
    return (m_texture->textureId() - m->m_texture->textureId());
}

StreamVideoNode::StreamVideoNode(QSGTexture *texture)
    : m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
{
    setGeometry(&m_geometry);
    setFlag(QSGNode::OwnsMaterial);
    m_material = new StreamVideoMaterial(texture);
    setMaterial(m_material);
}

void StreamVideoNode::setRect(const QRectF &rect)
{
    QSGGeometry::updateTexturedRectGeometry(geometry(), rect, QRectF(0, 0, 1, 1));
}
