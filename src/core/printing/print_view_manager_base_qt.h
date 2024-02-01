// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#ifndef PRINT_VIEW_MANAGER_BASE_QT_H
#define PRINT_VIEW_MANAGER_BASE_QT_H

#include "base/memory/ref_counted_memory.h"
#include "chrome/browser/printing/print_job.h"
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
class PrintQueriesQueue;
class PrinterQuery;
}

namespace QtWebEngineCore {
class PrintViewManagerBaseQt : public printing::PrintManager
                             , public printing::PrintJob::Observer
{
public:
    ~PrintViewManagerBaseQt() override;

    std::u16string RenderSourceName();

    // mojom::PrintManagerHost:
    void DidGetPrintedPagesCount(int32_t cookie, uint32_t number_pages) override;
    void DidPrintDocument(printing::mojom::DidPrintDocumentParamsPtr params,
                          DidPrintDocumentCallback callback) override;
    void GetDefaultPrintSettings(GetDefaultPrintSettingsCallback callback) override;
    void UpdatePrintSettings(int32_t cookie, base::Value::Dict job_settings,
                             UpdatePrintSettingsCallback callback) override;
    void IsPrintingEnabled(IsPrintingEnabledCallback callback) override;
    void ScriptedPrint(printing::mojom::ScriptedPrintParamsPtr,
                       printing::mojom::PrintManagerHost::ScriptedPrintCallback) override;
    void PrintingFailed(int32_t cookie,
                        printing::mojom::PrintFailureReason reason) override;

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

    // PrintJob::Observer overrides:
    void OnDocDone(int job_id, printing::PrintedDocument *document) override;
    void OnJobDone() override;
    void OnFailed() override;

    void StopWorker(int documentCookie);

private:
    // Requests the RenderView to render all the missing pages for the print job.
    // No-op if no print job is pending. Returns true if at least one page has
    // been requested to the renderer.
    bool RenderAllMissingPagesNow();

    // Runs `callback` with `params` to reply to ScriptedPrint().
    void ScriptedPrintReply(ScriptedPrintCallback callback,
                            int process_id,
                            printing::mojom::PrintPagesParamsPtr params);

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
    // function is called in DidPrintDocument(). The inner message loop was
    // created by RenderAllMissingPagesNow().
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

    base::WeakPtrFactory<PrintViewManagerBaseQt> weak_ptr_factory_{this};
};

} // namespace QtWebEngineCore
#endif // PRINT_VIEW_MANAGER_BASE_QT_H

