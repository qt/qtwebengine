// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "ozone_util_qt.h"

#include <QtGui/qguiapplication.h>
#include <qpa/qplatformnativeinterface.h>

#include <drm_fourcc.h>
#include <iomanip>
#include <sstream>
#include <xf86drm.h>

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
    if (modifier == DRM_FORMAT_MOD_LINEAR)
        return "DRM_FORMAT_MOD_LINEAR";

    if (modifier == DRM_FORMAT_MOD_INVALID)
        return "DRM_FORMAT_MOD_INVALID";

    // Vendor
    std::stringstream ss;
    if (char *vendorName = drmGetFormatModifierVendor(modifier)) {
        ss << "[" << vendorName << "]";
        free(vendorName);
    }

    // Modifier Code
    if (char *modifierName = drmGetFormatModifierName(modifier)) {
        // Unknown Vendor
        if (ss.tellp() <= 0) {
            uint8_t vendor = fourcc_mod_get_vendor(modifier);
            ss << "[0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(vendor)
               << "]";
        }

        // Delimiter
        ss << " ";

        // Modifier Name
        ss << modifierName;
        free(modifierName);
    } else {
        // Delimiter
        if (ss.tellp() > 0)
            ss << " ";

        // Raw Modifier
        ss << "0x" << std::hex << std::setfill('0') << std::setw(16) << modifier;
    }

    return ss.str();
}

} // namespace OzoneUtilQt

QT_END_NAMESPACE
