/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

#include "content_client_qt.h"

#include "base/command_line.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/user_agent.h"
#include "ui/base/layout.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "type_conversion.h"

#include <QCoreApplication>
#include <QFile>

#if defined(ENABLE_PLUGINS)
#include "content/public/common/pepper_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

static const int32 kPepperFlashPermissions = ppapi::PERMISSION_DEV |
                                             ppapi::PERMISSION_PRIVATE |
                                             ppapi::PERMISSION_BYPASS_USER_GESTURE |
                                             ppapi::PERMISSION_FLASH;

namespace switches {
const char kPpapiFlashPath[]    = "ppapi-flash-path";
const char kPpapiFlashVersion[] = "ppapi-flash-version";
}

// Adopted from chrome_content_client.cc
content::PepperPluginInfo CreatePepperFlashInfo(const base::FilePath& path, const std::string& version)
{
    content::PepperPluginInfo plugin;

    plugin.is_out_of_process = true;
    plugin.name = content::kFlashPluginName;
    plugin.path = path;
    plugin.permissions = kPepperFlashPermissions;

    std::vector<std::string> flash_version_numbers;
    base::SplitString(version, '.', &flash_version_numbers);
    if (flash_version_numbers.size() < 1)
        flash_version_numbers.push_back("11");
    else if (flash_version_numbers[0].empty())
        flash_version_numbers[0] = "11";
    if (flash_version_numbers.size() < 2)
        flash_version_numbers.push_back("2");
    if (flash_version_numbers.size() < 3)
        flash_version_numbers.push_back("999");
    if (flash_version_numbers.size() < 4)
        flash_version_numbers.push_back("999");

    // E.g., "Shockwave Flash 10.2 r154":
    plugin.description = plugin.name + " " + flash_version_numbers[0] + "." + flash_version_numbers[1] + " r" + flash_version_numbers[2];
    plugin.version = JoinString(flash_version_numbers, '.');
    content::WebPluginMimeType swf_mime_type(content::kFlashPluginSwfMimeType,
                                             content::kFlashPluginSwfExtension,
                                             content::kFlashPluginSwfDescription);
    plugin.mime_types.push_back(swf_mime_type);
    content::WebPluginMimeType spl_mime_type(content::kFlashPluginSplMimeType,
                                             content::kFlashPluginSplExtension,
                                             content::kFlashPluginSplDescription);
    plugin.mime_types.push_back(spl_mime_type);

    return plugin;
}

void AddPepperFlashFromSystem(std::vector<content::PepperPluginInfo>* plugins)
{
    QStringList pluginPaths;
#if defined(Q_OS_WIN)
    QString winDir = QDir::fromNativeSeparators(qgetenv("WINDIR"));
    if (winDir.isEmpty())
        winDir = QString::fromLatin1("C:/Windows");
    QDir pluginDir(winDir + "/System32/Macromed/Flash");
    pluginDir.setFilter(QDir::Files);
    Q_FOREACH (const QFileInfo &info, pluginDir.entryInfoList(QStringList("pepflashplayer*.dll")))
        pluginPaths << info.absoluteFilePath();
#endif
#if defined(Q_OS_OSX)
    pluginPaths << "/Library/Internet Plug-Ins/PepperFlashPlayer/PepperFlashPlayer.plugin"; // Mac OS X
#endif
#if defined(Q_OS_LINUX)
    pluginPaths << "/opt/google/chrome/PepperFlash/libpepflashplayer.so" // Google Chrome
                << "/usr/lib/pepperflashplugin-nonfree/libpepflashplayer.so" // Ubuntu
                << "/usr/lib/PepperFlash/libpepflashplayer.so" // Arch
                << "/usr/lib64/chromium/PepperFlash/libpepflashplayer.so"; // OpenSuSE
#endif
    std::string flash_version = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kPpapiFlashVersion);
    for (auto it = pluginPaths.constBegin(); it != pluginPaths.constEnd(); ++it) {
        if (!QFile::exists(*it))
            continue;
        plugins->push_back(CreatePepperFlashInfo(QtWebEngineCore::toFilePath(*it), flash_version));
    }
}

void AddPepperFlashFromCommandLine(std::vector<content::PepperPluginInfo>* plugins)
{
    const base::CommandLine::StringType flash_path = base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(switches::kPpapiFlashPath);
    if (flash_path.empty() || !QFile::exists(QtWebEngineCore::toQt(flash_path)))
        return;

    // Read pepper flash plugin version from command-line. (e.g. 16.0.0.235)
    std::string flash_version = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kPpapiFlashVersion);
    plugins->push_back(CreatePepperFlashInfo(base::FilePath(flash_path), flash_version));
}

namespace QtWebEngineCore {

void ContentClientQt::AddPepperPlugins(std::vector<content::PepperPluginInfo>* plugins)
{
    AddPepperFlashFromSystem(plugins);
    AddPepperFlashFromCommandLine(plugins);
}

}
#endif

#include <QCoreApplication>

namespace QtWebEngineCore {

std::string ContentClientQt::getUserAgent()
{
    // Mention the Chromium version we're based on to get passed stupid UA-string-based feature detection (several WebRTC demos need this)
    return content::BuildUserAgentFromProduct("QtWebEngine/" QTWEBENGINECORE_VERSION_STR " Chrome/" CHROMIUM_VERSION);
}

base::StringPiece ContentClientQt::GetDataResource(int resource_id, ui::ScaleFactor scale_factor) const {
    return ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(resource_id, scale_factor);
}

base::string16 ContentClientQt::GetLocalizedString(int message_id) const
{
    return l10n_util::GetStringUTF16(message_id);
}

std::string ContentClientQt::GetProduct() const
{
    QString productName(qApp->applicationName() % '/' % qApp->applicationVersion());
    return productName.toStdString();
}

} // namespace QtWebEngineCore
