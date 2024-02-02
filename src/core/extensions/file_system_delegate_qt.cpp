// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "file_system_delegate_qt.h"

#include "select_file_dialog_factory_qt.h"
#include "type_conversion.h"

#include "base/files/file_path.h"
#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/functional/callback.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/api/file_system/file_system_delegate.h"
#include "extensions/browser/extension_function.h"
#include "ui/shell_dialogs/selected_file_info.h"

#include <QStandardPaths>

namespace extensions {

FileEntryPickerQt::FileEntryPickerQt(
        content::WebContents *web_contents,
        const base::FilePath &suggested_name,
        const ui::SelectFileDialog::FileTypeInfo *file_type_info,
        ui::SelectFileDialog::Type picker_type,
        FileSystemDelegate::FilesSelectedCallback files_selected_callback,
        base::OnceClosure file_selection_canceled_callback)
    : m_filesSelectedCallback(std::move(files_selected_callback))
    , m_fileSelectionCanceledCallback(std::move(file_selection_canceled_callback))
{
    const GURL caller = web_contents->GetPrimaryMainFrame()->GetLastCommittedURL();
    m_selectFileDialog = ui::SelectFileDialog::Create(
        this, std::make_unique<QtWebEngineCore::SelectFilePolicyQt>(web_contents));
    m_selectFileDialog->SelectFile(
        picker_type, std::u16string(), suggested_name, file_type_info, 0,
        base::FilePath::StringType(), nullptr, nullptr, &caller);
}

FileEntryPickerQt::~FileEntryPickerQt() = default;

void FileEntryPickerQt::FileSelected(const base::FilePath &path,
        int index,
        void *params)
{
    MultiFilesSelected({path}, params);
}

void FileEntryPickerQt::FileSelectedWithExtraInfo(const ui::SelectedFileInfo& file,
        int index,
        void *params)
{
    FileSelected(file.file_path, index, params);
}

void FileEntryPickerQt::MultiFilesSelected(const std::vector<base::FilePath>& files,
                      void* params)
{
    Q_UNUSED(params);
    std::move(m_filesSelectedCallback).Run(files);
    delete this;
}

void FileEntryPickerQt::MultiFilesSelectedWithExtraInfo(
        const std::vector<ui::SelectedFileInfo> &files,
        void *params)
{
    std::vector<base::FilePath> paths;
    for (const auto& file : files)
        paths.push_back(file.file_path);
    MultiFilesSelected(paths, params);
}

void FileEntryPickerQt::FileSelectionCanceled(void *params)
{
    std::move(m_fileSelectionCanceledCallback).Run();
    delete this;
}

FileSystemDelegateQt::FileSystemDelegateQt()
{
}

base::FilePath FileSystemDelegateQt::GetDefaultDirectory()
{
    return QtWebEngineCore::toFilePath(
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

base::FilePath FileSystemDelegateQt::GetManagedSaveAsDirectory(
        content::BrowserContext *browser_context,
        const Extension &extension)
{
    Q_UNUSED(browser_context);
    Q_UNUSED(extension);
    return base::FilePath();
}

bool FileSystemDelegateQt::ShowSelectFileDialog(
        scoped_refptr<ExtensionFunction> extension_function,
        ui::SelectFileDialog::Type type,
        const base::FilePath &default_path,
        const ui::SelectFileDialog::FileTypeInfo *file_type_info,
        FileSystemDelegate::FilesSelectedCallback files_selected_callback,
        base::OnceClosure file_selection_canceled_callback)
{
    content::WebContents *web_contents = extension_function->GetSenderWebContents();
    if (!web_contents)
        return false;

    new FileEntryPickerQt(web_contents, default_path, file_type_info, type,
            std::move(files_selected_callback),
            std::move(file_selection_canceled_callback));
    return true;
}

void FileSystemDelegateQt::ConfirmSensitiveDirectoryAccess(
        bool has_write_permission,
        const std::u16string &app_name,
        content::WebContents *web_contents,
        base::OnceClosure on_accept,
        base::OnceClosure on_cancel)
{
    Q_UNUSED(has_write_permission);
    Q_UNUSED(app_name);
    Q_UNUSED(web_contents);
    Q_UNUSED(on_accept);
    std::move(on_cancel).Run();
}

int FileSystemDelegateQt::GetDescriptionIdForAcceptType(const std::string &accept_type)
{
    Q_UNUSED(accept_type);
    return 0;
}

SavedFilesServiceInterface *FileSystemDelegateQt::GetSavedFilesService(
        content::BrowserContext *browser_context)
{
    Q_UNUSED(browser_context);
    return nullptr;
}

} // namespace extensions
