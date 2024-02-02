// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef FILE_SYSTEM_DELEGATE_QT_H
#define FILE_SYSTEM_DELEGATE_QT_H

#include "extensions/browser/api/file_system/file_system_delegate.h"

#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/memory/ref_counted.h"
#include "extensions/browser/extension_function.h"
#include "ui/shell_dialogs/select_file_dialog.h"

#include <memory>
#include <vector>

namespace content {
class BrowserContext;
}  // namespace content

namespace extensions {

class FileEntryPickerQt : public ui::SelectFileDialog::Listener {
public:
    FileEntryPickerQt(
        content::WebContents *web_contents,
        const base::FilePath &suggested_name,
        const ui::SelectFileDialog::FileTypeInfo *file_type_info,
        ui::SelectFileDialog::Type picker_type,
        FileSystemDelegate::FilesSelectedCallback files_selected_callback,
        base::OnceClosure file_selection_canceled_callback);

    FileEntryPickerQt(const FileEntryPickerQt &) = delete;
    FileEntryPickerQt &operator=(const FileEntryPickerQt &) = delete;

private:
    ~FileEntryPickerQt() override;

    // ui::SelectFileDialog::Listener implementation.
    void FileSelected(const base::FilePath &path,
        int index,
        void *params) override;
    void FileSelectedWithExtraInfo(const ui::SelectedFileInfo &file,
        int index,
        void *params) override;
    void MultiFilesSelected(const std::vector<base::FilePath> &files,
                          void *params) override;
    void MultiFilesSelectedWithExtraInfo(
        const std::vector<ui::SelectedFileInfo> &files,
        void *params) override;
    void FileSelectionCanceled(void *params) override;

    FileSystemDelegate::FilesSelectedCallback m_filesSelectedCallback;
    base::OnceClosure m_fileSelectionCanceledCallback;
    scoped_refptr<ui::SelectFileDialog> m_selectFileDialog;
};

class FileSystemDelegateQt : public FileSystemDelegate
{
public:
    FileSystemDelegateQt();

    // FileSystemDelegate implementation
    virtual base::FilePath GetDefaultDirectory() override;
    virtual base::FilePath GetManagedSaveAsDirectory(
        content::BrowserContext *browser_context,
        const Extension &extension) override;
    virtual bool ShowSelectFileDialog(
        scoped_refptr<ExtensionFunction> extension_function,
        ui::SelectFileDialog::Type type,
        const base::FilePath &default_path,
        const ui::SelectFileDialog::FileTypeInfo *file_types,
        FileSystemDelegate::FilesSelectedCallback files_selected_callback,
        base::OnceClosure file_selection_canceled_callback) override;
    virtual void ConfirmSensitiveDirectoryAccess(
        bool has_write_permission,
        const std::u16string &app_name,
        content::WebContents *web_contents,
        base::OnceClosure on_accept,
        base::OnceClosure on_cancel) override;
    virtual int GetDescriptionIdForAcceptType(const std::string &accept_type) override;
    virtual SavedFilesServiceInterface *GetSavedFilesService(
        content::BrowserContext *browser_context) override;
};

} // namespace extensions

#endif // FILE_SYSTEM_DELEGATE_QT_H
