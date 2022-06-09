// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SELECT_FILE_DIALOG_FACTORY_QT_H_
#define SELECT_FILE_DIALOG_FACTORY_QT_H_

#include "ui/shell_dialogs/select_file_dialog_factory.h"
#include "ui/shell_dialogs/select_file_policy.h"

namespace content {
class WebContents;
}

namespace QtWebEngineCore {

class SelectFilePolicyQt : public ui::SelectFilePolicy
{
public:
    explicit SelectFilePolicyQt(content::WebContents *source_contents);
    ~SelectFilePolicyQt() override;

    // Overridden from ui::SelectFilePolicy:
    bool CanOpenSelectFileDialog() override;
    void SelectFileDenied() override;

    content::WebContents *webContents();

private:
    content::WebContents *m_webContents;
};

// Implements a file Open / Save dialog for File System Access API.
class SelectFileDialogFactoryQt : public ui::SelectFileDialogFactory
{
public:
    SelectFileDialogFactoryQt();
    ~SelectFileDialogFactoryQt() override;

    // Overridden from ui::SelectFileDialogFactory:
    ui::SelectFileDialog *Create(ui::SelectFileDialog::Listener *listener,
                                 std::unique_ptr<ui::SelectFilePolicy> policy) override;
};

} // namespace QtWebEngineCore

#endif // SELECT_FILE_DIALOG_FACTORY_QT_H_
