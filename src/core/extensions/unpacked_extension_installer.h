// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef UNPACKED_EXTENSION_INSTALLER_H_
#define UNPACKED_EXTENSION_INSTALLER_H_

#include <string>

#include "extension_loader.h"

#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/memory/ref_counted.h"
#include "base/task/sequenced_task_runner.h"

namespace QtWebEngineCore {
class UnpackedExtensionInstaller : public base::RefCountedThreadSafe<UnpackedExtensionInstaller>
{
public:
    using DoneCallback = base::OnceCallback<void(const base::FilePath &src,
                                                 const base::FilePath &extensionsInstallDir,
                                                 const std::string &error)>;
    static scoped_refptr<UnpackedExtensionInstaller>
    Create(const scoped_refptr<base::SequencedTaskRunner> &taskRunner, DoneCallback doneCallback);

    struct InstallInfo
    {
        std::string error{};
        base::FilePath extensionInstallPath{};
    };

    void install(const base::FilePath &src, const base::FilePath &installDir);
    static InstallInfo installUnpackedExtensionOnFileThread(const base::FilePath &src,
                                                            const base::FilePath &installDir);

private:
    UnpackedExtensionInstaller(const scoped_refptr<base::SequencedTaskRunner> &taskRunner,
                               DoneCallback doneCallback);
    void installInternal(const base::FilePath &src, const base::FilePath &installDir,
                         const ExtensionLoader::LoadingInfo &loadingInfo);
    void installDone(const base::FilePath &src, const InstallInfo &installInfo);

    scoped_refptr<base::SequencedTaskRunner> m_taskRunner;
    DoneCallback m_doneCallback;
};
} // namespace QtWebEngineCore
#endif // UNPACKED_EXTENSION_INSTALLER_H_
