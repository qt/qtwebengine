// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "select_file_dialog.h"

#include "ui/shell_dialogs/selected_file_info.h"
#include "ui/shell_dialogs/select_file_policy.h"
#include "content/public/browser/web_contents.h"

#include "content/public/browser/content_browser_client.h"
#include "content/public/common/content_client.h"

namespace content {
extern ContentClient *GetContentClient();
}

SelectFileDialog::~SelectFileDialog()
{
    select_file_dialog_->ListenerDestroyed();
}

void SelectFileDialog::Show(SelectedCallback selected_callback, const base::FilePath &default_path,
                            content::WebContents *web_contents)
{
    // Dialog is self-deleting.
    auto *dialog = new SelectFileDialog();
    dialog->ShowDialog(std::move(selected_callback), default_path, web_contents);
}

void SelectFileDialog::FileSelected(const ui::SelectedFileInfo &file, int index)
{
    std::move(selected_callback_).Run(file.path());
    delete this;
}

void SelectFileDialog::ShowDialog(SelectedCallback selected_callback,
                                  const base::FilePath &default_path,
                                  content::WebContents *web_contents)
{
    selected_callback_ = std::move(selected_callback);

    select_file_dialog_ = ui::SelectFileDialog::Create(
            this,
            std::unique_ptr<ui::SelectFilePolicy>(
                    content::GetContentClient()->browser()->CreateSelectFilePolicy(web_contents)));

    base::FilePath::StringType ext;
    ui::SelectFileDialog::FileTypeInfo file_type_info;
    select_file_dialog_->SelectFile(ui::SelectFileDialog::SELECT_FOLDER, std::u16string(),
                                    default_path, &file_type_info, 0, ext, {}, nullptr);
}
