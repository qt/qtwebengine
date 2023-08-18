// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "file_picker_controller.h"
#include "type_conversion.h"

#include "base/files/file_path.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/file_select_listener.h"
#include "ui/shell_dialogs/select_file_dialog.h"

#include <QtCore/qcoreapplication.h>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>
#include <QVariant>

namespace QtWebEngineCore {

class FilePickerControllerPrivate {
public:
    FilePickerController::FileChooserMode mode;
    scoped_refptr<content::FileSelectListener> fileDialogListener;
    ui::SelectFileDialog::Listener *fileSystemAccessDialogListener;
    QString defaultFileName;
    QStringList acceptedMimeTypes;
};

FilePickerController *createFilePickerController(
        FilePickerController::FileChooserMode mode, scoped_refptr<content::FileSelectListener> listener,
        const QString &defaultFileName, const QStringList &acceptedMimeTypes, QObject *parent = nullptr)
{
    auto priv = new FilePickerControllerPrivate { mode, listener, nullptr, defaultFileName,
                                                  acceptedMimeTypes };
    return new FilePickerController(priv, parent);
}

FilePickerController *createFilePickerController(FilePickerController::FileChooserMode mode,
                                                 ui::SelectFileDialog::Listener *listener,
                                                 const QString &defaultFileName,
                                                 const QStringList &acceptedMimeTypes,
                                                 QObject *parent = nullptr)
{
    auto priv = new FilePickerControllerPrivate { mode, nullptr, listener, defaultFileName,
                                                  acceptedMimeTypes };
    return new FilePickerController(priv, parent);
}

FilePickerController::FilePickerController(FilePickerControllerPrivate *priv, QObject *parent)
    : QObject(parent)
    , d_ptr(priv)
{
}

FilePickerController::~FilePickerController()
{
    if (!m_isHandled) {
        rejected();
    }
    delete d_ptr;
}

void FilePickerController::accepted(const QStringList &files)
{
    QStringList stringList;
    stringList.reserve(files.count());

    for (const QString &urlString : files) {
        // We accept strings on both absolute-path and file-URL form:
        if (toFilePath(urlString).IsAbsolute()) {
            stringList.append(urlString);
            continue;
        }

        if (urlString.startsWith("file:")) {
            base::FilePath filePath = toFilePath(urlString).NormalizePathSeparators();
            std::vector<base::FilePath::StringType> pathComponents;
            // Splits the file URL into scheme, host name, path and file name.
            pathComponents = filePath.GetComponents();

            QString absolutePath;
#if !defined(Q_OS_WIN)
            absolutePath = "/";
#endif

            QString scheme = toQt(pathComponents[0]);
            if (scheme.size() > 5) {
#if defined(Q_OS_WIN)
                // There is no slash at the end of the file scheme and it is valid on Windows: file:C:/
                if (scheme.size() == 7 && scheme.at(5).isLetter() && scheme.at(6) == ':') {
                    absolutePath += scheme.at(5) + ":/";
                } else {
#endif
                    qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
                    continue;
#if defined(Q_OS_WIN)
                }
#endif
            }

            // Non-local file and UNC Path validation: file://path/file
            if (base::FilePath::IsSeparator(urlString.at(5).toLatin1())
                && base::FilePath::IsSeparator(urlString.at(6).toLatin1())
                && !base::FilePath::IsSeparator(urlString.at(7).toLatin1())) {
#if defined(Q_OS_WIN)
                if (urlString.at(8) != ':' && pathComponents.size() > 2) {
                    absolutePath += "//";
#else
                if (pathComponents.size() > 2) {
                    absolutePath += "/";
#endif
                } else {
                    qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
                    continue;
                }
            }

            // Build absolute path from file URI componenets.
            for (size_t j = 1; j < pathComponents.size(); j++)
                absolutePath += toQt(pathComponents[j]) + (j != pathComponents.size()-1 ? "/" : "");

            if (toFilePath(absolutePath).IsAbsolute()) {
                stringList.append(absolutePath);
                continue;
            }
        }
        qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
    }

    FilePickerController::filesSelectedInChooser(stringList);
}

void FilePickerController::accepted(const QVariant &files)
{
    QStringList stringList;

    if (files.canConvert(QMetaType{QMetaType::QStringList})) {
        stringList = files.toStringList();
    } else if (files.canConvert<QList<QUrl> >()) {
        const QList<QUrl> urls = files.value<QList<QUrl>>();
        for (const QUrl &url : urls)
            stringList.append(url.toLocalFile());
    } else {
        qWarning("An unhandled type '%s' was provided in FilePickerController::accepted(QVariant)", files.typeName());
    }

    accepted(stringList);
}

void FilePickerController::rejected()
{
    FilePickerController::filesSelectedInChooser(QStringList());
}

static QStringList listRecursively(const QDir &dir)
{
    QStringList ret;
    const QFileInfoList infoList(dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden));
    for (const QFileInfo &fileInfo : infoList) {
        if (fileInfo.isDir()) {
            ret.append(fileInfo.absolutePath() + QStringLiteral("/.")); // Match chromium's behavior. See chrome/browser/file_select_helper.cc
            ret.append(listRecursively(QDir(fileInfo.absoluteFilePath())));
        } else
            ret.append(fileInfo.absoluteFilePath());
    }
    return ret;
}

ASSERT_ENUMS_MATCH(FilePickerController::Open, blink::mojom::FileChooserParams_Mode::kOpen)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, blink::mojom::FileChooserParams_Mode::kOpenMultiple)
ASSERT_ENUMS_MATCH(FilePickerController::UploadFolder, blink::mojom::FileChooserParams_Mode::kUploadFolder)
ASSERT_ENUMS_MATCH(FilePickerController::Save, blink::mojom::FileChooserParams_Mode::kSave)

void FilePickerController::filesSelectedInChooser(const QStringList &filesList)
{
    m_isHandled = true;
    if (d_ptr->fileDialogListener) {
        QStringList files(filesList);
        base::FilePath baseDir;
        if (d_ptr->mode == UploadFolder && !filesList.isEmpty()) {
            if (QFileInfo(filesList.first()).isDir()) {
                // Enumerate the directory
                files = listRecursively(QDir(filesList.first()));
                baseDir = toFilePath(filesList.first());
            } else {
                baseDir = toFilePath(filesList.first()).DirName();
            }
        }

        std::vector<blink::mojom::FileChooserFileInfoPtr> chooser_files;
        for (const auto &file : std::as_const(files)) {
            chooser_files.push_back(blink::mojom::FileChooserFileInfo::NewNativeFile(
                    blink::mojom::NativeFileInfo::New(toFilePath(file), std::u16string())));
        }

        if (files.isEmpty())
            d_ptr->fileDialogListener->FileSelectionCanceled();
        else
            d_ptr->fileDialogListener->FileSelected(
                    std::move(chooser_files), baseDir,
                    static_cast<blink::mojom::FileChooserParams::Mode>(d_ptr->mode));

        // release the fileSelectListener manually because it blocks fullscreen requests in chromium
        // see QTBUG-106975
        d_ptr->fileDialogListener.reset();
    } else if (d_ptr->fileSystemAccessDialogListener) {
        std::vector<base::FilePath> files;
        for (const auto &file : std::as_const(filesList)) {
            files.push_back(toFilePath(file));
        }

        if (files.empty())
            d_ptr->fileSystemAccessDialogListener->FileSelectionCanceled(nullptr);
        else
            d_ptr->fileSystemAccessDialogListener->MultiFilesSelected(files, nullptr);
    }
}

QStringList FilePickerController::acceptedMimeTypes() const
{
    return d_ptr->acceptedMimeTypes;
}

FilePickerController::FileChooserMode FilePickerController::mode() const
{
    return d_ptr->mode;
}

QString FilePickerController::defaultFileName() const
{
    return d_ptr->defaultFileName;
}

QStringList FilePickerController::nameFilters(const QStringList &acceptedMimeTypes)
{
    QStringList nameFilters;
    QStringList acceptedGlobs;
    QMimeDatabase mimeDatabase;

    if (acceptedMimeTypes.isEmpty())
        return nameFilters;

    for (QString type : acceptedMimeTypes) {
        if (type.startsWith(".")) {
            // A single suffix
            // Filename.type doesn't have to exist and mimeTypeForFile() supports
            // custom suffixes as valid (but unknown) MIME types.
            const QMimeType &mimeType = mimeDatabase.mimeTypeForFile("filename" + type);
            if (mimeType.isValid()) {
                QString glob = "*" + type;
                acceptedGlobs.append(glob);
                nameFilters.append(mimeType.comment() + " (" + glob + ")");
            }
        } else if (type.contains("/") && !type.endsWith("*")) {
            // All suffixes for a given MIME type
            const QMimeType &mimeType = mimeDatabase.mimeTypeForName(type);
            if (mimeType.isValid() && !mimeType.globPatterns().isEmpty()) {
                QString globs = mimeType.globPatterns().join(" ");
                acceptedGlobs.append(mimeType.globPatterns());
                nameFilters.append(mimeType.comment() + " (" + globs + ")");
            }
        } else if (type.endsWith("/*")) {
            // All MIME types for audio/*, image/* or video/*
            // as separate filters as Chrome does
            static const QList<QMimeType> &allMimeTypes = mimeDatabase.allMimeTypes();
            type = type.remove("/*");
            for (const QMimeType &m : allMimeTypes) {
                if (m.name().startsWith(type) && !m.globPatterns().isEmpty()) {
                    QString globs = m.globPatterns().join(" ");
                    acceptedGlobs.append(m.globPatterns());
                    nameFilters.append(m.comment() + " (" + globs + ")");
                }
            }
        } else {
            NOTREACHED();
        }
    }

    const QString filter =
        QCoreApplication::translate("FilePickerController",
                                    "Accepted types (%1)").arg(acceptedGlobs.join(' '));
    nameFilters.prepend(filter);

    return nameFilters;
}

} // namespace
