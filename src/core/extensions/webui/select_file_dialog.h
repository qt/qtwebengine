// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef SELECT_FILE_DIALOG_H
#define SELECT_FILE_DIALOG_H

#include "ui/shell_dialogs/select_file_dialog.h"
#include "base/functional/callback.h"

typedef base::OnceCallback<void(const base::FilePath &)> SelectedCallback;

namespace content {
class WebContents;
}
class SelectFileDialog : public ui::SelectFileDialog::Listener
{
public:
    SelectFileDialog(const SelectFileDialog &) = delete;
    SelectFileDialog &operator=(const SelectFileDialog &) = delete;

    static void Show(SelectedCallback selected_callback, const base::FilePath &default_path,
                     content::WebContents *web_contents);

    // ui::SelectFileDialog::Listener
    void FileSelected(const ui::SelectedFileInfo &file, int index) override;
    void FileSelectionCanceled() override { }

private:
    SelectFileDialog() = default;
    ~SelectFileDialog() override;

    void ShowDialog(SelectedCallback selected_callback, const base::FilePath &default_path,
                    content::WebContents *web_contents);

    scoped_refptr<ui::SelectFileDialog> select_file_dialog_;
    SelectedCallback selected_callback_;
};
#endif // SELECT_FILE_DIALOG_H
