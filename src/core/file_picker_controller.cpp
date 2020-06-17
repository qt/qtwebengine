/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "file_picker_controller.h"
#include "type_conversion.h"

#include "base/files/file_path.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/file_select_listener.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>
#include <QVariant>

namespace QtWebEngineCore {

FilePickerController::FilePickerController(FileChooserMode mode, std::unique_ptr<content::FileSelectListener> listener, const QString &defaultFileName, const QStringList &acceptedMimeTypes, QObject *parent)
    : QObject(parent)
    , m_defaultFileName(defaultFileName)
    , m_acceptedMimeTypes(acceptedMimeTypes)
    , m_listener(std::move(listener))
    , m_mode(mode)
{
}

FilePickerController::~FilePickerController() = default;

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
            // Splits the file URL into host name, path and file name.
            filePath.GetComponents(&pathComponents);

            QString absolutePath;
#if !defined(OS_WIN)
            absolutePath = "/";
#endif

            QString scheme = toQt(pathComponents[0]);
            if (scheme.size() > 5) {
#if defined(OS_WIN)
                // There is no slash at the end of the file scheme and it is valid on Windows: file:C:/
                if (scheme.at(5).isLetter() && scheme.at(6) != ':') {
                    absolutePath += scheme.at(5) + ":/";
                } else {
#endif
                    qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
                    continue;
#if defined(OS_WIN)
                }
#endif
            }

            // Non-local file and UNC Path validation: file://path/file
            if (base::FilePath::IsSeparator(urlString.at(5).toLatin1())
                && base::FilePath::IsSeparator(urlString.at(6).toLatin1())
                && !base::FilePath::IsSeparator(urlString.at(7).toLatin1())) {
#if defined(OS_WIN)
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

    if (files.canConvert(QVariant::StringList)) {
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
    QStringList files(filesList);
    if (this->m_mode == UploadFolder && !filesList.isEmpty()
            && QFileInfo(filesList.first()).isDir()) // Enumerate the directory
        files = listRecursively(QDir(filesList.first()));

    std::vector<blink::mojom::FileChooserFileInfoPtr> chooser_files;
    for (const auto &file : qAsConst(files)) {
        chooser_files.push_back(blink::mojom::FileChooserFileInfo::NewNativeFile(
            blink::mojom::NativeFileInfo::New(toFilePath(file), base::string16())));
    }

    if (files.isEmpty())
        m_listener->FileSelectionCanceled();
    else
        m_listener->FileSelected(std::move(chooser_files),
                                  /* FIXME? */ base::FilePath(),
                                 static_cast<blink::mojom::FileChooserParams::Mode>(this->m_mode));
}

QStringList FilePickerController::acceptedMimeTypes() const
{
    return m_acceptedMimeTypes;
}

FilePickerController::FileChooserMode FilePickerController::mode() const
{
    return m_mode;
}

QString FilePickerController::defaultFileName() const
{
    return m_defaultFileName;
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
                acceptedGlobs.append(globs);
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
                    acceptedGlobs.append(globs);
                    nameFilters.append(m.comment() + " (" + globs + ")");
                }
            }
        } else {
            NOTREACHED();
        }
    }

    nameFilters.prepend(QObject::tr("Accepted types") + " (" + acceptedGlobs.join(" ") + ")");

    return nameFilters;
}

} // namespace
