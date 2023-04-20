// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "select_file_dialog_factory_qt.h"

#include "content/browser/web_contents/web_contents_impl.h"
#include "file_picker_controller.h"
#include "type_conversion.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

#include <QSharedPointer>
#include <QTimer>

namespace QtWebEngineCore {

SelectFilePolicyQt::SelectFilePolicyQt(content::WebContents *source_contents)
    : m_webContents(source_contents)
{
}

SelectFilePolicyQt::~SelectFilePolicyQt() { }

bool SelectFilePolicyQt::CanOpenSelectFileDialog()
{
    return true;
}

void SelectFilePolicyQt::SelectFileDenied() { }

content::WebContents *SelectFilePolicyQt::webContents()
{
    return m_webContents;
}

static FilePickerController::FileChooserMode toFileChooserMode(ui::SelectFileDialog::Type type)
{
    switch (type) {
    case ui::SelectFileDialog::Type::SELECT_FOLDER:
    case ui::SelectFileDialog::Type::SELECT_UPLOAD_FOLDER:
    case ui::SelectFileDialog::Type::SELECT_EXISTING_FOLDER:
        return FilePickerController::FileChooserMode::UploadFolder;
    case ui::SelectFileDialog::Type::SELECT_SAVEAS_FILE:
        return FilePickerController::FileChooserMode::Save;
    case ui::SelectFileDialog::Type::SELECT_OPEN_FILE:
        return FilePickerController::FileChooserMode::Open;
    case ui::SelectFileDialog::Type::SELECT_OPEN_MULTI_FILE:
    case ui::SelectFileDialog::Type::SELECT_NONE:
    default:
        return FilePickerController::FileChooserMode::OpenMultiple;
        break;
    }
}

class SelectFileDialogQt : public ui::SelectFileDialog
{
public:
    SelectFileDialogQt(Listener *, std::unique_ptr<ui::SelectFilePolicy>,
                       WebContentsAdapterClient *);

    // ui::SelectFileDialog implementation:
    bool IsRunning(gfx::NativeWindow) const override;
    void ListenerDestroyed() override;
    void SelectFileImpl(Type type, const std::u16string &title, const base::FilePath &default_path,
                        const FileTypeInfo *file_types, int file_type_index,
                        const base::FilePath::StringType &default_extension,
                        gfx::NativeWindow owning_window, void *params, const GURL *) override;

    bool HasMultipleFileTypeChoicesImpl() override;

private:
    WebContentsAdapterClient *m_client;
    QSharedPointer<FilePickerController> m_filePickerController;
};

SelectFileDialogQt::SelectFileDialogQt(Listener *listener,
                                       std::unique_ptr<ui::SelectFilePolicy> policy,
                                       WebContentsAdapterClient *client)
    : SelectFileDialog(listener, std::move(policy)), m_client(client)
{
}

bool SelectFileDialogQt::IsRunning(gfx::NativeWindow) const
{
    return listener_;
}

void SelectFileDialogQt::ListenerDestroyed()
{
    listener_ = nullptr;
}

bool SelectFileDialogQt::HasMultipleFileTypeChoicesImpl()
{
    return false;
}

extern FilePickerController *createFilePickerController(FilePickerController::FileChooserMode mode,
                                                        ui::SelectFileDialog::Listener *listener,
                                                        const QString &defaultFileName,
                                                        const QStringList &acceptedMimeTypes,
                                                        QObject *parent = nullptr);

void SelectFileDialogQt::SelectFileImpl(Type type, const std::u16string &title,
                                        const base::FilePath &default_path,
                                        const FileTypeInfo *file_types, int file_type_index,
                                        const base::FilePath::StringType &default_extension,
                                        gfx::NativeWindow owning_window, void *params, const GURL *caller)
{
    Q_UNUSED(title);
    Q_UNUSED(file_type_index);
    Q_UNUSED(default_extension);
    Q_UNUSED(owning_window);
    Q_UNUSED(params);
    Q_UNUSED(caller);

    QStringList acceptedSuffixes;
    if (file_types) {
        for (const auto &type : file_types->extensions) {
            for (const auto &extension : type)
                acceptedSuffixes.append("." + toQt(extension));
        }
    }

    m_filePickerController.reset(createFilePickerController(
            toFileChooserMode(type), listener_, toQt(default_path.value()), acceptedSuffixes));
    QTimer::singleShot(0, [this]() { m_client->runFileChooser(m_filePickerController); });
}

SelectFileDialogFactoryQt::SelectFileDialogFactoryQt() = default;

SelectFileDialogFactoryQt::~SelectFileDialogFactoryQt() = default;

ui::SelectFileDialog *
SelectFileDialogFactoryQt::Create(ui::SelectFileDialog::Listener *listener,
                                  std::unique_ptr<ui::SelectFilePolicy> policy)
{
    content::WebContents *webContents =
            static_cast<SelectFilePolicyQt *>(policy.get())->webContents()->GetOutermostWebContents();
    WebContentsAdapterClient *client =
            WebContentsViewQt::from(static_cast<content::WebContentsImpl *>(webContents)->GetView())
                    ->client();
    if (!client)
        return nullptr;
    return new SelectFileDialogQt(listener, std::move(policy), client);
}

} // namespace QtWebEngineCore
