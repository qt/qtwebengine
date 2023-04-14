// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This file is based on chrome/browser/file_system_access/chrome_file_system_access_permission_context.cc:
// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "file_system_access_permission_context_qt.h"

#include "base/base_paths.h"
#include "base/path_service.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "chrome/common/chrome_paths.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "content/browser/web_contents/web_contents_impl.h"

#include "file_system_access_permission_grant_qt.h"
#include "type_conversion.h"

#include <QCoreApplication>
#include <QStandardPaths>

namespace QtWebEngineCore {

// Sentinel used to indicate that no PathService key is specified for a path in
// the struct below.
constexpr const int kNoBasePathKey = -1;

enum BlockType { kBlockAllChildren, kBlockNestedDirectories, kDontBlockChildren, kDontBlockAppFolder };

const struct
{
    // base::BasePathKey value (or one of the platform specific extensions to it)
    // for a path that should be blocked. Specify kNoBasePathKey if |path| should
    // be used instead.
    int base_path_key;

    // Explicit path to block instead of using |base_path_key|. Set to nullptr to
    // use |base_path_key| on its own. If both |base_path_key| and |path| are set,
    // |path| is treated relative to the path |base_path_key| resolves to.
    const base::FilePath::CharType *path;

    // If this is set to kDontBlockChildren, only the given path and its parents
    // are blocked. If this is set to kBlockAllChildren, all children of the given
    // path are blocked as well. Finally if this is set to kBlockNestedDirectories
    // access is allowed to individual files in the directory, but nested
    // directories are still blocked.
    // The BlockType of the nearest ancestor of a path to check is what ultimately
    // determines if a path is blocked or not. If a blocked path is a descendent
    // of another blocked path, then it may override the child-blocking policy of
    // its ancestor. For example, if /home blocks all children, but
    // /home/downloads does not, then /home/downloads/file.ext will *not* be
    // blocked.
    BlockType type;
} kBlockedPaths[] = {
    // Don't allow users to share their entire home directory, entire desktop or
    // entire documents folder, but do allow sharing anything inside those
    // directories not otherwise blocked.
    { base::DIR_HOME, nullptr, kDontBlockChildren },
    { base::DIR_USER_DESKTOP, nullptr, kDontBlockChildren },
    { chrome::DIR_USER_DOCUMENTS, nullptr, kDontBlockChildren },
    // Similar restrictions for the downloads directory.
    { chrome::DIR_DEFAULT_DOWNLOADS, nullptr, kDontBlockChildren },
    { chrome::DIR_DEFAULT_DOWNLOADS_SAFE, nullptr, kDontBlockChildren },
    // The Chrome installation itself should not be modified by the web.
    { base::DIR_EXE, nullptr, kBlockAllChildren },
    { base::DIR_MODULE, nullptr, kBlockAllChildren },
    { base::DIR_ASSETS, nullptr, kBlockAllChildren },
    // And neither should the configuration of at least the currently running
    // Chrome instance (note that this does not take --user-data-dir command
    // line overrides into account).
    { chrome::DIR_USER_DATA, nullptr, kBlockAllChildren },
    // ~/.ssh is pretty sensitive on all platforms, so block access to that.
    { base::DIR_HOME, FILE_PATH_LITERAL(".ssh"), kBlockAllChildren },
    // And limit access to ~/.gnupg as well.
    { base::DIR_HOME, FILE_PATH_LITERAL(".gnupg"), kBlockAllChildren },
#if defined(OS_WIN)
    // Some Windows specific directories to block, basically all apps, the
    // operating system itself, as well as configuration data for apps.
    { base::DIR_PROGRAM_FILES, nullptr, kBlockAllChildren },
    { base::DIR_PROGRAM_FILESX86, nullptr, kBlockAllChildren },
    { base::DIR_PROGRAM_FILES6432, nullptr, kBlockAllChildren },
    { base::DIR_WINDOWS, nullptr, kBlockAllChildren },
    { base::DIR_ROAMING_APP_DATA, nullptr, kBlockAllChildren },
    { base::DIR_LOCAL_APP_DATA, nullptr, kBlockAllChildren },
    // Whitelist AppData\Local\Temp to make the default location of QDir::tempPath(),
    // QTemporaryFile and QTemporaryDir working on Windows.
    { base::DIR_LOCAL_APP_DATA, FILE_PATH_LITERAL("Temp"), kDontBlockAppFolder },
    { base::DIR_COMMON_APP_DATA, nullptr, kBlockAllChildren },
    // Opening a file from an MTP device, such as a smartphone or a camera, is
    // implemented by Windows as opening a file in the temporary internet files
    // directory. To support that, allow opening files in that directory, but
    // not whole directories.
    { base::DIR_IE_INTERNET_CACHE, nullptr, kBlockNestedDirectories },
#endif
#if defined(OS_MAC)
    // Similar Mac specific blocks.
    { base::DIR_APP_DATA, nullptr, kBlockAllChildren },
    { base::DIR_HOME, FILE_PATH_LITERAL("Library"), kBlockAllChildren },
    // Allow access to iCloud files.
    { base::DIR_HOME, FILE_PATH_LITERAL("Library/Mobile Documents"), kDontBlockChildren },
#endif
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
    // On Linux also block access to devices via /dev, as well as security
    // sensitive data in /sys and /proc.
    { kNoBasePathKey, FILE_PATH_LITERAL("/dev"), kBlockAllChildren },
    { kNoBasePathKey, FILE_PATH_LITERAL("/sys"), kBlockAllChildren },
    { kNoBasePathKey, FILE_PATH_LITERAL("/proc"), kBlockAllChildren },
    // And block all of ~/.config, matching the similar restrictions on mac
    // and windows.
    { base::DIR_HOME, FILE_PATH_LITERAL(".config"), kBlockAllChildren },
    // Block ~/.dbus as well, just in case, although there probably isn't much a
    // website can do with access to that directory and its contents.
    { base::DIR_HOME, FILE_PATH_LITERAL(".dbus"), kBlockAllChildren },
#endif
    // TODO(https://crbug.com/984641): Refine this list, for example add
    // XDG_CONFIG_HOME when it is not set ~/.config?
};

bool ShouldBlockAccessToPath(const base::FilePath &check_path, HandleType handle_type)
{
    DCHECK(!check_path.empty());
    DCHECK(check_path.IsAbsolute());

    base::FilePath nearest_ancestor;
    int nearest_ancestor_path_key = kNoBasePathKey;
    BlockType nearest_ancestor_block_type = kDontBlockChildren;
    for (const auto &block : kBlockedPaths) {
        base::FilePath blocked_path;
        if (block.base_path_key != kNoBasePathKey) {
            if (!base::PathService::Get(block.base_path_key, &blocked_path))
                continue;
            if (block.path)
                blocked_path = blocked_path.Append(block.path);
        } else {
            DCHECK(block.path);
            blocked_path = base::FilePath(block.path);
        }

        if (check_path == blocked_path || check_path.IsParent(blocked_path)) {
            VLOG(1) << "Blocking access to " << check_path << " because it is a parent of "
                    << blocked_path << " (" << block.base_path_key << ")";
            return true;
        }

        if (blocked_path.IsParent(check_path)
            && (nearest_ancestor.empty() || nearest_ancestor.IsParent(blocked_path))) {
            nearest_ancestor = blocked_path;
            nearest_ancestor_path_key = block.base_path_key;
            nearest_ancestor_block_type = block.type;
        }
    }

    // The path we're checking is not in a potentially blocked directory, or the
    // nearest ancestor does not block access to its children. Grant access.
    if (nearest_ancestor.empty() || nearest_ancestor_block_type == kDontBlockChildren)
        return false;

    // The path we're checking is a file, and the nearest ancestor only blocks
    // access to directories. Grant access.
    if (handle_type == HandleType::kFile && nearest_ancestor_block_type == kBlockNestedDirectories)
        return false;

    if (nearest_ancestor_block_type == kDontBlockAppFolder) {
        // Relative path from the nearest blocklisted ancestor
        base::FilePath diff;
        nearest_ancestor.AppendRelativePath(check_path, &diff);

        auto diff_components = diff.GetComponents();
        if (diff_components.size() > 0 && toQt(diff_components[0]).contains(QCoreApplication::applicationName())) {
            // The relative path contains the application name. Grant access.
            return false;
        }
    }

    // The nearest ancestor blocks access to its children, so block access.
    VLOG(1) << "Blocking access to " << check_path << " because it is inside " << nearest_ancestor
            << " (" << nearest_ancestor_path_key << ")";
    return true;
}

struct FileSystemAccessPermissionContextQt::OriginState
{
    // Raw pointers, owned collectively by all the handles that reference this
    // grant. When last reference goes away this state is cleared as well by
    // PermissionGrantDestroyed().
    std::map<base::FilePath, FileSystemAccessPermissionGrantQt *> read_grants;
    std::map<base::FilePath, FileSystemAccessPermissionGrantQt *> write_grants;
};

FileSystemAccessPermissionContextQt::FileSystemAccessPermissionContextQt(
        content::BrowserContext *context)
    : m_profile(context)
{
}

FileSystemAccessPermissionContextQt::~FileSystemAccessPermissionContextQt() = default;

scoped_refptr<content::FileSystemAccessPermissionGrant>
FileSystemAccessPermissionContextQt::GetReadPermissionGrant(const url::Origin &origin,
                                                            const base::FilePath &path,
                                                            HandleType handle_type,
                                                            UserAction user_action)
{
    auto &origin_state = m_origins[origin];
    auto *&existing_grant = origin_state.read_grants[path];
    scoped_refptr<FileSystemAccessPermissionGrantQt> new_grant;

    if (existing_grant && existing_grant->handleType() != handle_type) {
        // |path| changed from being a directory to being a file or vice versa,
        // don't just re-use the existing grant but revoke the old grant before
        // creating a new grant.
        existing_grant->SetStatus(blink::mojom::PermissionStatus::DENIED);
        existing_grant = nullptr;
    }

    if (!existing_grant) {
        new_grant = base::MakeRefCounted<FileSystemAccessPermissionGrantQt>(
                m_weakFactory.GetWeakPtr(), origin, path, handle_type, GrantType::kRead);
        existing_grant = new_grant.get();
    }

    // If a parent directory is already readable this new grant should also be readable.
    if (new_grant && AncestorHasActivePermission(origin, path, GrantType::kRead)) {
        existing_grant->SetStatus(blink::mojom::PermissionStatus::GRANTED);
        return existing_grant;
    }

    switch (user_action) {
    case UserAction::kOpen:
    case UserAction::kSave:
        // Open and Save dialog only grant read access for individual files.
        if (handle_type == HandleType::kDirectory)
            break;
        Q_FALLTHROUGH();
    case UserAction::kDragAndDrop:
        // Drag&drop grants read access for all handles.
        existing_grant->SetStatus(blink::mojom::PermissionStatus::GRANTED);
        break;
    case UserAction::kLoadFromStorage:
        break;
    case UserAction::kNone:
        Q_UNREACHABLE();
    }

    return existing_grant;
}

scoped_refptr<content::FileSystemAccessPermissionGrant>
FileSystemAccessPermissionContextQt::GetWritePermissionGrant(const url::Origin &origin,
                                                             const base::FilePath &path,
                                                             HandleType handle_type,
                                                             UserAction user_action)
{
    auto &origin_state = m_origins[origin];
    auto *&existing_grant = origin_state.write_grants[path];
    scoped_refptr<FileSystemAccessPermissionGrantQt> new_grant;

    if (existing_grant && existing_grant->handleType() != handle_type) {
        // |path| changed from being a directory to being a file or vice versa,
        // don't just re-use the existing grant but revoke the old grant before
        // creating a new grant.
        existing_grant->SetStatus(blink::mojom::PermissionStatus::DENIED);
        existing_grant = nullptr;
    }

    if (!existing_grant) {
        new_grant = base::MakeRefCounted<FileSystemAccessPermissionGrantQt>(
                m_weakFactory.GetWeakPtr(), origin, path, handle_type, GrantType::kWrite);
        existing_grant = new_grant.get();
    }

    // If a parent directory is already writable this new grant should also be writable.
    if (new_grant && AncestorHasActivePermission(origin, path, GrantType::kWrite)) {
        existing_grant->SetStatus(blink::mojom::PermissionStatus::GRANTED);
        return existing_grant;
    }

    switch (user_action) {
    case UserAction::kSave:
        // Only automatically grant write access for save dialogs.
        existing_grant->SetStatus(blink::mojom::PermissionStatus::GRANTED);
        break;
    case UserAction::kOpen:
    case UserAction::kDragAndDrop:
    case UserAction::kLoadFromStorage:
        break;
    case UserAction::kNone:
        Q_UNREACHABLE();
    }

    return existing_grant;
}

void FileSystemAccessPermissionContextQt::ConfirmSensitiveEntryAccess(
        const url::Origin &origin, PathType path_type, const base::FilePath &path,
        HandleType handle_type, UserAction user_action,
        content::GlobalRenderFrameHostId frame_id,
        base::OnceCallback<void(SensitiveEntryResult)> callback)
{
    if (path_type == PathType::kExternal) {
        std::move(callback).Run(SensitiveEntryResult::kAllowed);
        return;
    }

    base::ThreadPool::PostTaskAndReplyWithResult(
            FROM_HERE, { base::MayBlock(), base::TaskPriority::USER_VISIBLE },
            base::BindOnce(&ShouldBlockAccessToPath, path, handle_type),
            base::BindOnce(&FileSystemAccessPermissionContextQt::DidConfirmSensitiveDirectoryAccess,
                           m_weakFactory.GetWeakPtr(), origin, path, handle_type, user_action, frame_id,
                           std::move(callback)));
}

void FileSystemAccessPermissionContextQt::PerformAfterWriteChecks(
        std::unique_ptr<content::FileSystemAccessWriteItem> item,
        content::GlobalRenderFrameHostId frame_id,
        base::OnceCallback<void(AfterWriteCheckResult)> callback)
{
    Q_UNUSED(item);
    Q_UNUSED(frame_id);
    std::move(callback).Run(AfterWriteCheckResult::kAllow);
}

bool FileSystemAccessPermissionContextQt::CanObtainReadPermission(const url::Origin &origin)
{
    Q_UNUSED(origin);
    return true;
}

bool FileSystemAccessPermissionContextQt::CanObtainWritePermission(const url::Origin &origin)
{
    Q_UNUSED(origin);
    return true;
}

void FileSystemAccessPermissionContextQt::SetLastPickedDirectory(const url::Origin &origin,
                                                                 const std::string &id,
                                                                 const base::FilePath &path,
                                                                 const PathType type)
{
    Q_UNUSED(origin);

    FileSystemAccessPermissionContextQt::PathInfo info;
    info.path = path;
    info.type = type;
    m_lastPickedDirectories.insert({ id, info });
}

FileSystemAccessPermissionContextQt::PathInfo
FileSystemAccessPermissionContextQt::GetLastPickedDirectory(const url::Origin &origin,
                                                            const std::string &id)
{
    Q_UNUSED(origin);

    return m_lastPickedDirectories.find(id) != m_lastPickedDirectories.end()
            ? m_lastPickedDirectories[id]
            : FileSystemAccessPermissionContextQt::PathInfo();
}

base::FilePath FileSystemAccessPermissionContextQt::GetWellKnownDirectoryPath(
        blink::mojom::WellKnownDirectory directory, const url::Origin &origin)
{
    QStandardPaths::StandardLocation location = QStandardPaths::DocumentsLocation;
    switch (directory) {
    case blink::mojom::WellKnownDirectory::kDefault:
        location = QStandardPaths::DocumentsLocation;
        break;
    case blink::mojom::WellKnownDirectory::kDirDesktop:
        location = QStandardPaths::DesktopLocation;
        break;
    case blink::mojom::WellKnownDirectory::kDirDocuments:
        location = QStandardPaths::DocumentsLocation;
        break;
    case blink::mojom::WellKnownDirectory::kDirDownloads:
        location = QStandardPaths::DownloadLocation;
        break;
    case blink::mojom::WellKnownDirectory::kDirMusic:
        location = QStandardPaths::MusicLocation;
        break;
    case blink::mojom::WellKnownDirectory::kDirPictures:
        location = QStandardPaths::PicturesLocation;
        break;
    case blink::mojom::WellKnownDirectory::kDirVideos:
        location = QStandardPaths::MoviesLocation;
        break;
    }

    return toFilePath(QStandardPaths::writableLocation(location));
}

void FileSystemAccessPermissionContextQt::NavigatedAwayFromOrigin(const url::Origin &origin)
{
    // If the last top-level WebContents for an origin is closed (or is navigated to another
    // origin), all the permissions for that origin will be revoked.

    auto it = m_origins.find(origin);
    // If we have no permissions for the origin, there is nothing to do.
    if (it == m_origins.end())
        return;

    std::vector<content::WebContentsImpl *> list = content::WebContentsImpl::GetAllWebContents();
    for (content::WebContentsImpl *web_contents : list) {
        url::Origin web_contents_origin = url::Origin::Create(web_contents->GetLastCommittedURL());
        // Found a tab for this origin, so early exit and don't revoke grants.
        if (web_contents_origin == origin)
            return;
    }

    OriginState &origin_state = it->second;
    for (auto &grant : origin_state.read_grants)
        grant.second->SetStatus(blink::mojom::PermissionStatus::ASK);
    for (auto &grant : origin_state.write_grants)
        grant.second->SetStatus(blink::mojom::PermissionStatus::ASK);
}

void FileSystemAccessPermissionContextQt::DidConfirmSensitiveDirectoryAccess(
        const url::Origin &origin, const base::FilePath &path, HandleType handle_type, UserAction user_action,
        content::GlobalRenderFrameHostId frame_id,
        base::OnceCallback<void(SensitiveEntryResult)> callback, bool should_block)
{
    Q_UNUSED(origin);
    Q_UNUSED(path);
    Q_UNUSED(handle_type);
    Q_UNUSED(user_action);
    Q_UNUSED(frame_id);

    if (should_block)
        std::move(callback).Run(SensitiveEntryResult::kAbort);
    else
        std::move(callback).Run(SensitiveEntryResult::kAllowed);
}

bool FileSystemAccessPermissionContextQt::AncestorHasActivePermission(
    const url::Origin &origin, const base::FilePath &path, GrantType grant_type) const
{
    auto it = m_origins.find(origin);
    if (it == m_origins.end())
        return false;

    const auto &relevant_grants = grant_type == GrantType::kWrite ? it->second.write_grants : it->second.read_grants;
    if (relevant_grants.empty())
        return false;

    // Permissions are inherited from the closest ancestor.
    for (base::FilePath parent = path.DirName(); parent != parent.DirName(); parent = parent.DirName()) {
        auto i = relevant_grants.find(parent);
        if (i != relevant_grants.end() && i->second && i->second->GetStatus() == blink::mojom::PermissionStatus::GRANTED)
            return true;
    }
    return false;
}

std::u16string FileSystemAccessPermissionContextQt::GetPickerTitle(const blink::mojom::FilePickerOptionsPtr &)
{
    return {};
}

void FileSystemAccessPermissionContextQt::PermissionGrantDestroyed(
        FileSystemAccessPermissionGrantQt *grant)
{
    auto it = m_origins.find(grant->origin());
    if (it == m_origins.end())
        return;

    auto &grants =
            grant->type() == GrantType::kRead ? it->second.read_grants : it->second.write_grants;
    auto grant_it = grants.find(grant->path());

    if (grant_it == grants.end()) {
        return;
    }
    if (grant_it->second == grant)
        grants.erase(grant_it);
}

void FileSystemAccessPermissionContextQt::NotifyEntryMoved(const url::Origin &, const base::FilePath &, const base::FilePath &)
{
}

} // namespace QtWebEngineCore
