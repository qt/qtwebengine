/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

    DISALLOW_COPY_AND_ASSIGN(SelectFilePolicyQt);
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

private:
    DISALLOW_COPY_AND_ASSIGN(SelectFileDialogFactoryQt);
};

} // namespace QtWebEngineCore

#endif // SELECT_FILE_DIALOG_FACTORY_QT_H_
