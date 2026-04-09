// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "web_contents_adapter_client.h"

#include "content/public/common/javascript_dialog_type.h"
#include "ui/base/window_open_disposition.h"

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::UnknownDisposition,             WindowOpenDisposition::UNKNOWN)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::CurrentTabDisposition,          WindowOpenDisposition::CURRENT_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::SingletonTabDisposition,        WindowOpenDisposition::SINGLETON_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::NewForegroundTabDisposition,    WindowOpenDisposition::NEW_FOREGROUND_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::NewBackgroundTabDisposition,    WindowOpenDisposition::NEW_BACKGROUND_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::NewPopupDisposition,            WindowOpenDisposition::NEW_POPUP)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::NewWindowDisposition,           WindowOpenDisposition::NEW_WINDOW)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::SaveToDiskDisposition,          WindowOpenDisposition::SAVE_TO_DISK)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::OffTheRecordDisposition,        WindowOpenDisposition::OFF_THE_RECORD)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::IgnoreActionDisposition,        WindowOpenDisposition::IGNORE_ACTION)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::SwitchToTabDisposition,         WindowOpenDisposition::SWITCH_TO_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::NewPictureInPictureDisposition, WindowOpenDisposition::NEW_PICTURE_IN_PICTURE)

ASSERT_ENUMS_MATCH(WebContentsAdapterClient::WindowOpenDisposition::NewPictureInPictureDisposition, WindowOpenDisposition::MAX_VALUE)

ASSERT_ENUMS_MATCH(WebContentsAdapterClient::JavascriptDialogType::AlertDialog,     content::JavaScriptDialogType::JAVASCRIPT_DIALOG_TYPE_ALERT)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::JavascriptDialogType::ConfirmDialog,   content::JavaScriptDialogType::JAVASCRIPT_DIALOG_TYPE_CONFIRM)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::JavascriptDialogType::PromptDialog,    content::JavaScriptDialogType::JAVASCRIPT_DIALOG_TYPE_PROMPT)
// No mathing Chromium enum for UnloadDialog
// No mathing Chromium enum for InternalAuthorizationDialog

}
