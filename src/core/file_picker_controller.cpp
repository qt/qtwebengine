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
#if defined(OS_WIN)
#include "base/files/file_path.h"
#endif
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/file_select_listener.h"

#include <QFileInfo>
#include <QDir>
#include <QVariant>
#include <QStringList>

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
        if (QDir::isAbsolutePath(urlString)) {
            QString absolutePath = QDir::fromNativeSeparators(urlString);
#if defined(OS_WIN)
            if (absolutePath.at(0).isLetter() && absolutePath.at(1) == QLatin1Char(':') && !base::FilePath::IsSeparator(absolutePath.at(2).toLatin1()))
                qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
            else
#endif
                stringList.append(absolutePath);
        } else {
            QUrl url(urlString, QUrl::StrictMode);
            if (url.isLocalFile() && QDir::isAbsolutePath(url.toLocalFile())) {
                QString absolutePath = url.toLocalFile();
#if defined(OS_WIN)
                if (absolutePath.at(0).isLetter() && absolutePath.at(1) == QLatin1Char(':') && !base::FilePath::IsSeparator(absolutePath.at(2).toLatin1()))
                    qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
                else
#endif
                    stringList.append(absolutePath);
            } else
                qWarning("Ignoring invalid item in FilePickerController::accepted(QStringList): %s", qPrintable(urlString));
        }
    }

    FilePickerController::filesSelectedInChooser(stringList);
}

void FilePickerController::accepted(const QVariant &files)
{
    if (!files.canConvert(QVariant::StringList))
        qWarning("An unhandled type '%s' was provided in FilePickerController::accepted(QVariant)", files.typeName());

    accepted(files.toStringList());
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

} // namespace
