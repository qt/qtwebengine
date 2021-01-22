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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#ifndef PRINT_VIEW_MANAGER_BASE_QT_H
#define PRINT_VIEW_MANAGER_BASE_QT_H

#include "base/memory/ref_counted_memory.h"
#include "base/strings/string16.h"
#include "components/prefs/pref_member.h"
#include "components/printing/browser/print_manager.h"
#include "components/printing/common/print.mojom-forward.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace base {
class RefCountedBytes;
}

namespace content {
class RenderFrameHost;
}

namespace printing {
class JobEventDetails;
class PrintJob;
class PrintQueriesQueue;
class PrinterQuery;
}

namespace QtWebEngineCore {
class PrintViewManagerBaseQt : public content::NotificationObserver
                             , public printing::PrintManager
{
public:
    ~PrintViewManagerBaseQt() override;

    // Whether printing is enabled or not.
    void UpdatePrintingEnabled();

    base::string16 RenderSourceName();

    // mojom::PrintManagerHost:
    void DidGetPrintedPagesCount(int32_t cookie, uint32_t number_pages) override;
    void GetDefaultPrintSettings(GetDefaultPrintSettingsCallback callback) override;
    void ShowInvalidPrinterSettingsError() override;
    void PrintingFailed(int32_t cookie) override;

protected:
    explicit PrintViewManagerBaseQt(content::WebContents*);

    void SetPrintingRFH(content::RenderFrameHost* rfh);

    // Cancels the print job.
    void NavigationStopped() override;

    // content::WebContentsObserver implementation.
    void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;

    // Creates a new empty print job. It has no settings loaded. If there is
    // currently a print job, safely disconnect from it. Returns false if it is
    // impossible to safely disconnect from the current print job or it is
    // impossible to create a new print job.
    virtual bool CreateNewPrintJob(std::unique_ptr<printing::PrinterQuery> query);

    // Makes sure the current print_job_ has all its data before continuing, and
    // disconnect from it.
    void DisconnectFromCurrentPrintJob();

    void StopWorker(int documentCookie);

private:
    // content::NotificationObserver implementation.
    void Observe(int,
                 const content::NotificationSource&,
                 const content::NotificationDetails&) override;

    // content::WebContentsObserver implementation.
    void DidStartLoading() override;

    // printing::PrintManager:
    void OnDidPrintDocument(
        content::RenderFrameHost *render_frame_host,
        const printing::mojom::DidPrintDocumentParams &params,
        std::unique_ptr<DelayedFrameDispatchHelper> helper) override;
    void OnScriptedPrint(content::RenderFrameHost *render_frame_host,
                         const printing::mojom::ScriptedPrintParams &params,
                         IPC::Message *reply_msg) override;

    // Processes a NOTIFY_PRINT_JOB_EVENT notification.
    void OnNotifyPrintJobEvent(const printing::JobEventDetails &event_details);

    // Requests the RenderView to render all the missing pages for the print job.
    // No-op if no print job is pending. Returns true if at least one page has
    // been requested to the renderer.
    bool RenderAllMissingPagesNow();

    // Checks that synchronization is correct with |print_job_| based on |cookie|.
    bool PrintJobHasDocument(int cookie);

    // Starts printing the |document| in |print_job_| with the given |print_data|.
    // This method assumes PrintJobHasDocument() has been called, and |print_data|
    // contains valid data.
    void PrintDocument(scoped_refptr<base::RefCountedMemory> print_data,
                       const gfx::Size &page_size,
                       const gfx::Rect &content_area,
                       const gfx::Point &offsets);

    // Quits the current message loop if these conditions hold true: a document is
    // loaded and is complete and waiting_for_pages_to_be_rendered_ is true. This
    // function is called in DidPrintDocument() or on ALL_PAGES_REQUESTED
    // notification. The inner message loop is created was created by
    // RenderAllMissingPagesNow().
    void ShouldQuitFromInnerMessageLoop();

    // Terminates the print job. No-op if no print job has been created. If
    // |cancel| is true, cancel it instead of waiting for the job to finish. Will
    // call ReleasePrintJob().
    void TerminatePrintJob(bool cancel);

    // Releases print_job_. Correctly deregisters from notifications. No-op if
    // no print job has been created.
    void ReleasePrintJob();

    // Runs an inner message loop. It will set inside_inner_message_loop_ to true
    // while the blocking inner message loop is running. This is useful in cases
    // where the RenderView is about to be destroyed while a printing job isn't
    // finished.
    bool RunInnerMessageLoop();

    // In the case of Scripted Printing, where the renderer is controlling the
    // control flow, print_job_ is initialized whenever possible. No-op is
    // print_job_ is initialized.
    bool OpportunisticallyCreatePrintJob(int cookie);

    // Release the PrinterQuery associated with our |cookie_|.
    void ReleasePrinterQuery();

    // Helper method for UpdatePrintingEnabled().
    void SendPrintingEnabled(bool enabled, content::RenderFrameHost* rfh);

private:
    content::NotificationRegistrar m_registrar;
    scoped_refptr<printing::PrintJob> m_printJob;
    content::RenderFrameHost *m_printingRFH;
    bool m_didPrintingSucceed;
    // Set while running an inner message loop inside RenderAllMissingPagesNow().
    // This means we are _blocking_ until all the necessary pages have been
    // rendered or the print settings are being loaded.
    base::OnceClosure m_quitInnerLoop;
    scoped_refptr<printing::PrintQueriesQueue> m_printerQueriesQueue;

    DISALLOW_COPY_AND_ASSIGN(PrintViewManagerBaseQt);
};

} // namespace QtWebEngineCore
#endif // PRINT_VIEW_MANAGER_BASE_QT_H

