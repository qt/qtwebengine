/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "content_main_delegate_qt.h"

#include "base/path_service.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/webui/jstemplate_builder.h"
#include "grit/net_resources.h"
#include "net/base/net_module.h"

#include "content_client_qt.h"
#include "web_engine_library_info.h"

#include<QObject>

namespace {

// FIXME: makes more sense to default string.
struct LazyDirectoryListerCacher {
  LazyDirectoryListerCacher() {
    base::DictionaryValue value;
    value.SetString("header", QObject::tr("Index of LOCATION").toStdString());
    value.SetString("parentDirText", QObject::tr("[parent directory]").toStdString());
    value.SetString("headerName", QObject::tr("Name").toStdString());
    value.SetString("headerSize", QObject::tr("size").toStdString());
    value.SetString("headerDateModified", QObject::tr("Date Modified").toStdString());
    value.SetString("listingParsingErrorBoxText", QObject::tr("Oh, no! This server is sending data I can't understand.").toStdString());

    html_data = webui::GetI18nTemplateHtml(
        ResourceBundle::GetSharedInstance().GetRawDataResource(
            IDR_DIR_HEADER_HTML),
        &value);
  }

  std::string html_data;
};

}  // namespace

static base::StringPiece PlatformResourceProvider(int key) {
    CR_DEFINE_STATIC_LOCAL(LazyDirectoryListerCacher, lazy_dir_lister, ());
    if (key == IDR_DIR_HEADER_HTML) {
        base::StringPiece html_data = base::StringPiece(lazy_dir_lister.html_data);
        return html_data;
    }
    return base::StringPiece();
}

void ContentMainDelegateQt::PreSandboxStartup()
{
    PathService::Override(base::FILE_EXE, WebEngineLibraryInfo::subProcessPath());
    PathService::Override(ui::DIR_LOCALES, WebEngineLibraryInfo::localesPath());

    net::NetModule::SetResourceProvider(PlatformResourceProvider);
    ui::ResourceBundle::InitSharedInstanceWithLocale(l10n_util::GetApplicationLocale(std::string("en-US")), 0);
}

content::ContentBrowserClient *ContentMainDelegateQt::CreateContentBrowserClient()
{
    m_browserClient.reset(new ContentBrowserClientQt);
    return m_browserClient.get();
}

bool ContentMainDelegateQt::BasicStartupComplete(int *exit_code)
{
    SetContentClient(new ContentClientQt);
    return false;
}

