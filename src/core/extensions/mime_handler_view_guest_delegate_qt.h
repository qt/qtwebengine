/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIME_HANDLER_VIEW_GUEST_DELEGATE_QT_H_
#define MIME_HANDLER_VIEW_GUEST_DELEGATE_QT_H_

#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest_delegate.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/public/browser/web_contents.h"

namespace content {
struct ContextMenuParams;
}

namespace extensions {
class MimeHandlerViewGuest;

class MimeHandlerViewGuestDelegateQt : public MimeHandlerViewGuestDelegate
{
public:
    explicit MimeHandlerViewGuestDelegateQt(MimeHandlerViewGuest *guest);
    ~MimeHandlerViewGuestDelegateQt() override;

    bool HandleContextMenu(content::WebContents *web_contents,
                           const content::ContextMenuParams &params) override;

private:
    DISALLOW_COPY_AND_ASSIGN(MimeHandlerViewGuestDelegateQt);
};

} // namespace extensions

#endif // MIME_HANDLER_VIEW_GUEST_DELEGATE_QT_H_
