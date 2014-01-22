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
#include "content/public/common/content_paths.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "grit/net_resources.h"
#include "net/base/net_module.h"

#include "content_client_qt.h"
#include "web_engine_library_info.h"

static base::StringPiece PlatformResourceProvider(int key) {
    if (key == IDR_DIR_HEADER_HTML) {
        base::StringPiece html_data = ui::ResourceBundle::GetSharedInstance().GetRawDataResource(IDR_DIR_HEADER_HTML);
        return html_data;
    }
    return base::StringPiece();
}

void ContentMainDelegateQt::PreSandboxStartup()
{
    PathService::Override(base::FILE_EXE, WebEngineLibraryInfo::subProcessPath());
    PathService::Override(content::DIR_MEDIA_LIBS, WebEngineLibraryInfo::pluginsPath());
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

