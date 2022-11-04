// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BUILD_CONFIG_QT
#define BUILD_CONFIG_QT

#include <QtCore/qglobal.h>
#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <QtWebEngineCore/private/qtwebenginecore-config_p.h>

#include "printing/buildflags/buildflags.h"
#include "components/spellcheck/spellcheck_buildflags.h"
#include "media/media_buildflags.h"
#include "ppapi/buildflags/buildflags.h"

// This is just config sanity check
#if QT_CONFIG(webengine_printing_and_pdf)
#if !BUILDFLAG(ENABLE_PRINTING) || !BUILDFLAG(ENABLE_PRINT_PREVIEW)
#error Config sanity check for webengine_printing_and_pdf failed
#endif
#else
#if BUILDFLAG(ENABLE_PRINTING) || BUILDFLAG(ENABLE_PRINT_PREVIEW)
#error Config sanity check for ENABLE_PRINTING, ENABLE_PRINT_PREVIEW failed
#endif
#endif

#if QT_CONFIG(webengine_spellchecker)
#if !BUILDFLAG(ENABLE_SPELLCHECK)
#error Config sanity check for webengine_spellchecker failed
#endif
#else
#if BUILDFLAG(ENABLE_SPELLCHECK)
#error Config sanity check for ENABLE_SPELLCHECK failed
#endif
#endif

#if QT_CONFIG(webengine_pepper_plugins)
#if !BUILDFLAG(ENABLE_PLUGINS)
#error Config sanity check for webengine_pepper_plugins failed
#endif
#else
#if BUILDFLAG(ENABLE_PLUGINS)
#error Config sanity check for ENABLE_PLUGINS failed
#endif
#endif

#if QT_CONFIG(webengine_webrtc)
#if !BUILDFLAG(ENABLE_WEBRTC)
#error Config sanity check for webengine_webrtc failed
#endif
#else
#if BUILDFLAG(ENABLE_WEBRTC)
#error Config sanity check for ENABLE_WEBRTC failed
#endif
#endif

#if QT_CONFIG(webengine_native_spellchecker)
#if !BUILDFLAG(USE_BROWSER_SPELLCHECKER)
#error Config sanity check for webengine_native_spellchecker failed
#endif
#else
#if BUILDFLAG(USE_BROWSER_SPELLCHECKER)
#error Config sanity check for USE_BROWSER_SPELLCHECKER failed
#endif
#endif

#endif
