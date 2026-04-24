// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "ozone_util_qt.h"

#include <QtGui/qguiapplication.h>
#include <qpa/qplatformnativeinterface.h>

#include <drm_fourcc.h>
#include <iomanip>
#include <sstream>

#if QT_CONFIG(opengl)
#include <QtGui/qopenglcontext.h>
#endif

QT_BEGIN_NAMESPACE

namespace OzoneUtilQt {
void *getXDisplay()
{
#if QT_CONFIG(xcb)
    auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (x11Application)
        return x11Application->display();
#endif

    return nullptr;
}

QOpenGLContext *getQOpenGLContext()
{
#if QT_CONFIG(opengl)
    if (auto *shareContext = QOpenGLContext::globalShareContext())
        return shareContext;

    if (auto *currentContext = QOpenGLContext::currentContext())
        return currentContext;

    static QOpenGLContext *tmpGLContext = []() {
        auto tmpGLContext = new QOpenGLContext();
        tmpGLContext->create();
        QObject::connect(qGuiApp, &QGuiApplication::aboutToQuit, [=]() { delete tmpGLContext; });
        return tmpGLContext;
    }();

    return tmpGLContext;
#else
    return nullptr;
#endif
}

bool usingGLX()
{
#if QT_CONFIG(opengl) && QT_CONFIG(xcb_glx_plugin)
    static bool result = []() {
        QOpenGLContext *context = getQOpenGLContext();
        return context->nativeInterface<QNativeInterface::QGLXContext>() != nullptr;
    }();

    return result;
#else
    return false;
#endif
}

bool usingEGL()
{
#if QT_CONFIG(opengl) && QT_CONFIG(egl)
    static bool result = []() {
        QOpenGLContext *context = getQOpenGLContext();
        return context->nativeInterface<QNativeInterface::QEGLContext>() != nullptr;
    }();

    return result;
#else
    return false;
#endif
}

std::string drmFormatModifierToString(const uint64_t modifier)
{
    auto formatHex = [](uint64_t val, int width) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(width) << val;
        return ss.str();
    };

    std::string hexValue = formatHex(modifier, 16);

    if (modifier == 0)
        return "[LINEAR] " + hexValue;

    if (modifier == DRM_FORMAT_MOD_INVALID)
        return "[INVALID] " + hexValue;

    std::string vendorTag;
    uint8_t vendor = fourcc_mod_get_vendor(modifier);
    switch (vendor) {
    case DRM_FORMAT_MOD_VENDOR_INTEL:
        vendorTag = "[INTEL]";
        break;
    case DRM_FORMAT_MOD_VENDOR_AMD:
        vendorTag = "[AMD]";
        break;
    case DRM_FORMAT_MOD_VENDOR_NVIDIA:
        vendorTag = "[NVIDIA]";
        break;
    case DRM_FORMAT_MOD_VENDOR_ARM:
        vendorTag = "[ARM]";
        break;
    default:
        vendorTag = formatHex(vendor, 2);
        break;
    }

    return vendorTag + " " + hexValue;
}

std::string fourccToString(const uint32_t fourcc)
{
    char buf[5];
    buf[0] = static_cast<char>(fourcc & 0xff);
    buf[1] = static_cast<char>((fourcc >> 8) & 0xff);
    buf[2] = static_cast<char>((fourcc >> 16) & 0xff);
    buf[3] = static_cast<char>((fourcc >> 24) & 0xff);
    buf[4] = 0;
    return std::string(buf);
}

} // namespace OzoneUtilQt

QT_END_NAMESPACE
