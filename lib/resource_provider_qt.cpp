#include "cc/resources/resource_provider.h"

#include <QDebug>
#include <QImage>

static inline quint32 swapBgrToRgb(quint32 pixel)
{
    return ((pixel << 16) & 0xff0000) | ((pixel >> 16) & 0xff) | (pixel & 0xff00ff00);
}

namespace cc {

void ResourceProvider::saveImage(ResourceProvider::ResourceId resource_id, int width, int height)
{
    ResourceMap::iterator it = resources_.find(resource_id);
    Resource* resource = &it->second;

    // Alternatively read pixels to a memory buffer.
    QImage offscreenImage(width, height, QImage::Format_ARGB32);
    quint32* imagePixels = reinterpret_cast<quint32*>(offscreenImage.bits());

    GLint previousFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

    GLuint myFB = 0;
    glGenFramebuffers(1, &myFB);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, myFB);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resource->gl_id, 0);
    glReadPixels(/* x */ 0, /* y */ 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imagePixels);

    // OpenGL gives us ABGR on 32 bits, and with the origin at the bottom left
    // We need RGB32 or ARGB32_PM, with the origin at the top left.
    quint32* pixelsSrc = imagePixels;
    const int halfHeight = height / 2;
    for (int row = 0; row < halfHeight; ++row) {
        const int targetIdx = (height - 1 - row) * width;
        quint32* pixelsDst = imagePixels + targetIdx;
        for (int column = 0; column < width; ++column) {
            quint32 tempPixel = *pixelsSrc;
            *pixelsSrc = swapBgrToRgb(*pixelsDst);
            *pixelsDst = swapBgrToRgb(tempPixel);
            ++pixelsSrc;
            ++pixelsDst;
        }
    }
    if (static_cast<int>(height) % 2) {
        for (int column = 0; column < width; ++column) {
            *pixelsSrc = swapBgrToRgb(*pixelsSrc);
            ++pixelsSrc;
        }
    }

    QString filename = QStringLiteral("/tmp/offscreen");
    static int suffix = 0;
    filename.append(QString::number(suffix++));
    filename.append(".png");
    if(!offscreenImage.save(filename))
        fprintf(stderr, "Failed to save to file!!!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
    glDeleteFramebuffers(1, &myFB);
}

}

