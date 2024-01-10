// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This is based on chrome/browser/printing/print_view_manager_base.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "print_view_manager_qt.h"

#include "type_conversion.h"
#include "web_engine_context.h"

#include "base/memory/ref_counted_memory.h"
#include "base/run_loop.h"
#include "base/task/current_thread.h"
#include "base/task/thread_pool.h"
#include "base/timer/timer.h"
#include "base/values.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/printing/print_job.h"
#include "chrome/browser/printing/print_job_manager.h"
#include "chrome/browser/printing/printer_query.h"
#include "components/printing/browser/print_manager_utils.h"
#include "components/printing/common/print.mojom.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "printing/metafile_skia.h"
#include "printing/print_job_constants.h"
#include "printing/printed_document.h"

namespace QtWebEngineCore {

namespace {

// Runs |callback| with |params| to reply to
// mojom::PrintManagerHost::GetDefaultPrintSettings.
void GetDefaultPrintSettingsReply(printing::mojom::PrintManagerHost::GetDefaultPrintSettingsCallback callback,
                                  printing::mojom::PrintParamsPtr params)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    std::move(callback).Run(std::move(params));
}

void OnDidGetDefaultPrintSettings(scoped_refptr<printing::PrintQueriesQueue> queue,
                                      std::unique_ptr<printing::PrinterQuery> printer_query,
                                      printing::mojom::PrintManagerHost::GetDefaultPrintSettingsCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    printing::mojom::PrintParamsPtr params = printing::mojom::PrintParams::New();
    if (printer_query && printer_query->last_status() == printing::mojom::ResultCode::kSuccess) {
        RenderParamsFromPrintSettings(printer_query->settings(), params.get());
        params->document_cookie = printer_query->cookie();
    }

    GetDefaultPrintSettingsReply(std::move(callback), std::move(params));

    // If printing was enabled.
    if (printer_query) {
        // If user hasn't cancelled.
        if (printer_query->cookie() && printer_query->settings().dpi()) {
            queue->QueuePrinterQuery(std::move(printer_query));
        }
    }
}

printing::mojom::PrintPagesParamsPtr CreateEmptyPrintPagesParamsPtr()
{
    auto params = printing::mojom::PrintPagesParams::New();
    params->params = printing::mojom::PrintParams::New();
    return params;
}

// Runs |callback| with |params| to reply to
// mojom::PrintManagerHost::UpdatePrintSettings.
void UpdatePrintSettingsReply(printing::mojom::PrintManagerHost::UpdatePrintSettingsCallback callback,
                              printing::mojom::PrintPagesParamsPtr params, bool canceled)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (!params)
        params = CreateEmptyPrintPagesParamsPtr();
    std::move(callback).Run(std::move(params), canceled);
}

void OnDidUpdatePrintSettings(scoped_refptr<printing::PrintQueriesQueue> queue,
                                  std::unique_ptr<printing::PrinterQuery> printer_query,
                                  printing::mojom::PrintManagerHost::UpdatePrintSettingsCallback callback,
                                  int process_id, int routing_id)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(printer_query);
    auto params = printing::mojom::PrintPagesParams::New();
    params->params = printing::mojom::PrintParams::New();
    if (printer_query->last_status() == printing::mojom::ResultCode::kSuccess) {
        RenderParamsFromPrintSettings(printer_query->settings(), params->params.get());
        params->params->document_cookie = printer_query->cookie();
        params->pages = printer_query->settings().ranges();
    }
    bool canceled = printer_query->last_status() == printing::mojom::ResultCode::kCanceled;

    UpdatePrintSettingsReply(std::move(callback), std::move(params), canceled);

    if (printer_query->cookie() && printer_query->settings().dpi()) {
        queue->QueuePrinterQuery(std::move(printer_query));
    }
}


void OnDidScriptedPrint(scoped_refptr<printing::PrintQueriesQueue> queue,
                            std::unique_ptr<printing::PrinterQuery> printer_query,
                            printing::mojom::PrintManagerHost::ScriptedPrintCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    printing::mojom::PrintPagesParamsPtr params = CreateEmptyPrintPagesParamsPtr();
    if (printer_query->last_status() == printing::mojom::ResultCode::kSuccess && printer_query->settings().dpi()) {
        RenderParamsFromPrintSettings(printer_query->settings(), params->params.get());
        params->params->document_cookie = printer_query->cookie();
        params->pages = printer_query->settings().ranges();
    }
    bool has_valid_cookie = params->params->document_cookie;
    bool has_dpi = !params->params->dpi.IsEmpty();
    std::move(callback).Run(std::move(params));

    if (has_dpi && has_valid_cookie) {
        queue->QueuePrinterQuery(std::move(printer_query));
    }
}

}  // namespace

PrintViewManagerBaseQt::PrintViewManagerBaseQt(content::WebContents *contents)
    : printing::PrintManager(contents)
    , m_printingRFH(nullptr)
    , m_didPrintingSucceed(false)
    , m_printerQueriesQueue(WebEngineContext::current()->getPrintJobManager()->queue())
{
}

PrintViewManagerBaseQt::~PrintViewManagerBaseQt()
{
    ReleasePrinterQuery();
    DisconnectFromCurrentPrintJob();
}

void PrintViewManagerBaseQt::SetPrintingRFH(content::RenderFrameHost *rfh)
{
    DCHECK(!m_printingRFH);
    m_printingRFH = rfh;
}

void PrintViewManagerBaseQt::ScriptedPrintReply(ScriptedPrintCallback callback,
                                                int process_id,
                                                printing::mojom::PrintPagesParamsPtr params) {
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

#if BUILDFLAG(ENABLE_OOP_PRINTING)
    // Finished getting all settings (defaults and from user), no further need
    // to be registered as a system print client.
    UnregisterSystemPrintClient();
#endif
    if (!content::RenderProcessHost::FromID(process_id)) {
        // Early return if the renderer is not alive.
        return;
    }

//    set_cookie(params->params->document_cookie);
    std::move(callback).Run(std::move(params));
}

void PrintViewManagerBaseQt::NavigationStopped()
{
    // Cancel the current job, wait for the worker to finish.
    TerminatePrintJob(true);
}

std::u16string PrintViewManagerBaseQt::RenderSourceName()
{
     return toString16(QLatin1String(""));
}

void PrintViewManagerBaseQt::PrintDocument(scoped_refptr<base::RefCountedMemory> print_data,
                                           const gfx::Size &page_size,
                                           const gfx::Rect &content_area,
                                           const gfx::Point &offsets)
{
    std::unique_ptr<printing::MetafileSkia> metafile =
            std::make_unique<printing::MetafileSkia>();
    CHECK(metafile->InitFromData(*print_data));

    // Update the rendered document. It will send notifications to the listener.
    printing::PrintedDocument* document = m_printJob->document();
    document->SetDocument(std::move(metafile));
    ShouldQuitFromInnerMessageLoop();
}

void PrintViewManagerBaseQt::DidGetPrintedPagesCount(int32_t cookie, uint32_t number_pages)
{
    PrintManager::DidGetPrintedPagesCount(cookie, number_pages);
    OpportunisticallyCreatePrintJob(cookie);
}

bool PrintViewManagerBaseQt::PrintJobHasDocument(int cookie)
{
    if (!OpportunisticallyCreatePrintJob(cookie))
        return false;

    // These checks may fail since we are completely asynchronous. Old spurious
    // messages can be received if one of the processes is overloaded.
    printing::PrintedDocument* document = m_printJob->document();
    return document && document->cookie() == cookie;
}

void PrintViewManagerBaseQt::DidPrintDocument(printing::mojom::DidPrintDocumentParamsPtr params,
                                              DidPrintDocumentCallback callback)
{
    if (!PrintJobHasDocument(params->document_cookie)) {
        std::move(callback).Run(false);
        return;
    }

    const printing::mojom::DidPrintContentParams &content = *params->content;
    if (!content.metafile_data_region.IsValid()) {
        NOTREACHED() << "invalid memory handle";
        web_contents()->Stop();
        std::move(callback).Run(false);
        return;
    }

    auto data = base::RefCountedSharedMemoryMapping::CreateFromWholeRegion(content.metafile_data_region);
    if (!data) {
        NOTREACHED() << "couldn't map";
        web_contents()->Stop();
        std::move(callback).Run(false);
        return;
    }

    PrintDocument(data, params->page_size, params->content_area,
                  params->physical_offsets);
    std::move(callback).Run(true);
}

void PrintViewManagerBaseQt::GetDefaultPrintSettings(GetDefaultPrintSettingsCallback callback)
{
    content::RenderFrameHost *render_frame_host =
            print_manager_host_receivers_.GetCurrentTargetFrame();
    content::RenderProcessHost *render_process_host =
            render_frame_host->GetProcess();

    auto callback_wrapper =
        base::BindOnce(&GetDefaultPrintSettingsReply, std::move(callback));
    std::unique_ptr<printing::PrinterQuery> printer_query = m_printerQueriesQueue->PopPrinterQuery(0);
    if (!printer_query)
        printer_query = m_printerQueriesQueue->CreatePrinterQuery(render_frame_host->GetGlobalId());

     // Loads default settings. This is asynchronous, only the mojo message sender
     // will hang until the settings are retrieved.
     auto *printer_query_ptr = printer_query.get();
     printer_query_ptr->GetDefaultSettings(
                 base::BindOnce(&OnDidGetDefaultPrintSettings, m_printerQueriesQueue,
                                std::move(printer_query), std::move(callback_wrapper)),
                 false,
                 !render_process_host->IsPdf());
}

void PrintViewManagerBaseQt::PrintingFailed(int32_t cookie, printing::mojom::PrintFailureReason reason)
{
    // Note: Not redundant with cookie checks in the same method in other parts of
    // the class hierarchy.
    if (!IsValidCookie(cookie))
        return;

    PrintManager::PrintingFailed(cookie, reason);

    ReleasePrinterQuery();
}
void PrintViewManagerBaseQt::IsPrintingEnabled(IsPrintingEnabledCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    bool enabled = false;
#if QT_CONFIG(webengine_printing_and_pdf)
    enabled = true;
#endif
    std::move(callback).Run(enabled);
}

void PrintViewManagerBaseQt::ScriptedPrint(printing::mojom::ScriptedPrintParamsPtr params,
                                           printing::mojom::PrintManagerHost::ScriptedPrintCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    content::RenderFrameHost *render_frame_host =
            print_manager_host_receivers_.GetCurrentTargetFrame();
    content::RenderProcessHost *render_process_host =
            render_frame_host->GetProcess();

    auto callback_wrapper = base::BindOnce(
        &PrintViewManagerBaseQt::ScriptedPrintReply, weak_ptr_factory_.GetWeakPtr(),
        std::move(callback), render_process_host->GetID());

    std::unique_ptr<printing::PrinterQuery> printer_query =
         m_printerQueriesQueue->PopPrinterQuery(params->cookie);
    if (!printer_query)
       printer_query = m_printerQueriesQueue->CreatePrinterQuery(render_frame_host->GetGlobalId());

    auto *printer_query_ptr = printer_query.get();
    printer_query_ptr->GetSettingsFromUser(
                params->expected_pages_count, params->has_selection, params->margin_type,
                params->is_scripted, !render_process_host->IsPdf(),
                base::BindOnce(&OnDidScriptedPrint, m_printerQueriesQueue, std::move(printer_query), std::move(callback_wrapper)));
}

// Note: In PrintViewManagerQt we always initiate printing with
// printing::mojom::PrintRenderFrame::InitiatePrintPreview()
// so m_printingRFH is never set and used at the moment.
void PrintViewManagerBaseQt::RenderFrameDeleted(content::RenderFrameHost *render_frame_host)
{
    PrintManager::RenderFrameDeleted(render_frame_host);

    // Terminates or cancels the print job if one was pending.
    if (render_frame_host != m_printingRFH)
        return;

    m_printingRFH = nullptr;

    PrintManager::PrintingRenderFrameDeleted();
    ReleasePrinterQuery();

    if (!m_printJob.get())
        return;

    scoped_refptr<printing::PrintedDocument> document(m_printJob->document());
    if (document) {
        // If IsComplete() returns false, the document isn't completely rendered.
        // Since our renderer is gone, there's nothing to do, cancel it. Otherwise,
        // the print job may finish without problem.
        TerminatePrintJob(!document->IsComplete());
    }
}

void PrintViewManagerBaseQt::OnDocDone(int /*job_id*/, printing::PrintedDocument * /*document*/)
{
}

void PrintViewManagerBaseQt::OnJobDone()
{
    // Printing is done, we don't need it anymore.
    // print_job_->is_job_pending() may still be true, depending on the order
    // of object registration.
    m_didPrintingSucceed = true;
    ReleasePrintJob();
}

void PrintViewManagerBaseQt::OnFailed()
{
    TerminatePrintJob(true);
}

// Requests the RenderView to render all the missing pages for the print job.
// No-op if no print job is pending. Returns true if at least one page has
// been requested to the renderer.
bool PrintViewManagerBaseQt::RenderAllMissingPagesNow()
{
    if (!m_printJob.get() || !m_printJob->is_job_pending())
      return false;

    // Is the document already complete?
    if (m_printJob->document() && m_printJob->document()->IsComplete()) {
        m_didPrintingSucceed = true;
        return true;
    }

    // We can't print if there is no renderer.
    if (!web_contents() ||
        !web_contents()->GetPrimaryMainFrame() ||
        !web_contents()->GetPrimaryMainFrame()->IsRenderFrameLive()) {
      return false;
    }

    // WebContents is either dying or a second consecutive request to print
    // happened before the first had time to finish. We need to render all the
    // pages in an hurry if a print_job_ is still pending. No need to wait for it
    // to actually spool the pages, only to have the renderer generate them. Run
    // a message loop until we get our signal that the print job is satisfied.
    // |quit_inner_loop_| will be called as soon as
    // print_job_->document()->IsComplete() is true in DidPrintDocument(). The
    // check is done in ShouldQuitFromInnerMessageLoop().
    // BLOCKS until all the pages are received. (Need to enable recursive task)
    if (!RunInnerMessageLoop()) {
      // This function is always called from DisconnectFromCurrentPrintJob() so we
      // know that the job will be stopped/canceled in any case.
      return false;
    }
    return true;
}

void PrintViewManagerBaseQt::ShouldQuitFromInnerMessageLoop()
{
    // Look at the reason.
    DCHECK(m_printJob->document());
    if (m_printJob->document() && m_printJob->document()->IsComplete() && m_quitInnerLoop) {
        // We are in a message loop created by RenderAllMissingPagesNow. Quit from
        // it.
        std::move(m_quitInnerLoop).Run();
    }
}

bool PrintViewManagerBaseQt::CreateNewPrintJob(std::unique_ptr<printing::PrinterQuery> query)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(!m_quitInnerLoop);
    DCHECK(query);

    // Disconnect the current |m_printJob|.
    auto weak_this = weak_ptr_factory_.GetWeakPtr();
    DisconnectFromCurrentPrintJob();
    if (!weak_this)
      return false;

    // We can't print if there is no renderer.
    if (!web_contents()->GetPrimaryMainFrame() ||
        !web_contents()->GetPrimaryMainFrame()->IsRenderFrameLive()) {
        return false;
    }

    // Ask the renderer to generate the print preview, create the print preview
    // view and switch to it, initialize the printer and show the print dialog.
    DCHECK(!m_printJob.get());

    m_printJob = base::MakeRefCounted<printing::PrintJob>(nullptr /*g_browser_process->print_job_manager()*/);
    m_printJob->Initialize(std::move(query), RenderSourceName(), number_pages_);
    m_printJob->AddObserver(*this);
    m_didPrintingSucceed = false;
    return true;
}

void PrintViewManagerBaseQt::DisconnectFromCurrentPrintJob()
{
    // Make sure all the necessary rendered page are done. Don't bother with the
    // return value.
    bool result = RenderAllMissingPagesNow();

    // Verify that assertion.
    if (m_printJob.get() &&
        m_printJob->document() &&
        !m_printJob->document()->IsComplete()) {
        DCHECK(!result);
        // That failed.
        TerminatePrintJob(true);
    } else {
        // DO NOT wait for the job to finish.
        ReleasePrintJob();
    }
}

void PrintViewManagerBaseQt::TerminatePrintJob(bool cancel)
{
    if (!m_printJob.get())
        return;

    if (cancel) {
        // We don't need the metafile data anymore because the printing is canceled.
        m_printJob->Cancel();
        m_quitInnerLoop.Reset();
    } else {
        DCHECK(!m_quitInnerLoop);
        DCHECK(!m_printJob->document() || m_printJob->document()->IsComplete());

        // WebContents is either dying or navigating elsewhere. We need to render
        // all the pages in an hurry if a print job is still pending. This does the
        // trick since it runs a blocking message loop:
        m_printJob->Stop();
    }
    ReleasePrintJob();
}

void PrintViewManagerBaseQt::ReleasePrintJob()
{
    content::RenderFrameHost* rfh = m_printingRFH;
    m_printingRFH = nullptr;

    if (!m_printJob.get())
        return;

    if (rfh)
      GetPrintRenderFrame(rfh)->PrintingDone(m_didPrintingSucceed);

    m_printJob->RemoveObserver(*this);

    // Don't close the worker thread.
    m_printJob = nullptr;
}

bool PrintViewManagerBaseQt::RunInnerMessageLoop()
{
  // This value may actually be too low:
  //
  // - If we're looping because of printer settings initialization, the premise
  // here is that some poor users have their print server away on a VPN over a
  // slow connection. In this situation, the simple fact of opening the printer
  // can be dead slow. On the other side, we don't want to die infinitely for a
  // real network error. Give the printer 60 seconds to comply.
  //
  // - If we're looping because of renderer page generation, the renderer could
  // be CPU bound, the page overly complex/large or the system just
  // memory-bound.
  static const int kPrinterSettingsTimeout = 60000;
  base::OneShotTimer quit_timer;
  base::RunLoop run_loop;
  quit_timer.Start(FROM_HERE,
                   base::Milliseconds(kPrinterSettingsTimeout),
                   run_loop.QuitWhenIdleClosure());

  m_quitInnerLoop = run_loop.QuitClosure();

  auto weak_this = weak_ptr_factory_.GetWeakPtr();
  run_loop.Run();
  if (!weak_this)
      return false;

  bool success = !m_quitInnerLoop;
  m_quitInnerLoop.Reset();

  return success;
}

bool PrintViewManagerBaseQt::OpportunisticallyCreatePrintJob(int cookie)
{
    if (m_printJob.get())
      return true;

    if (!cookie) {
      // Out of sync. It may happens since we are completely asynchronous. Old
      // spurious message can happen if one of the processes is overloaded.
      return false;
    }

    // The job was initiated by a script. Time to get the corresponding worker
    // thread.
    std::unique_ptr<printing::PrinterQuery> queued_query = m_printerQueriesQueue->PopPrinterQuery(cookie);
    if (!queued_query) {
      NOTREACHED();
      return false;
    }

    if (!CreateNewPrintJob(std::move(queued_query))) {
      // Don't kill anything.
      return false;
    }

    // Settings are already loaded. Go ahead. This will set
    // print_job_->is_job_pending() to true.
    m_printJob->StartPrinting();
    return true;
}

void PrintViewManagerBaseQt::ReleasePrinterQuery()
{
    if (!cookie_)
        return;

    int cookie = cookie_;
    cookie_ = 0;

    printing::PrintJobManager* printJobManager = WebEngineContext::current()->getPrintJobManager();
    // May be NULL in tests.
    if (!printJobManager)
        return;

    std::unique_ptr<printing::PrinterQuery> printerQuery =
            m_printerQueriesQueue->PopPrinterQuery(cookie);
}

// Originally from print_preview_message_handler.cc:
void PrintViewManagerBaseQt::StopWorker(int documentCookie)
{
    if (documentCookie <= 0)
        return;
    std::unique_ptr<printing::PrinterQuery> printerQuery =
            m_printerQueriesQueue->PopPrinterQuery(documentCookie);
}

void PrintViewManagerBaseQt::UpdatePrintSettings(int32_t cookie, base::Value::Dict job_settings,
                                                 UpdatePrintSettingsCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    if (!job_settings.FindInt(printing::kSettingPrinterType)) {
        UpdatePrintSettingsReply(std::move(callback), nullptr, false);
        return;
    }

    content::RenderFrameHost *render_frame_host =
            print_manager_host_receivers_.GetCurrentTargetFrame();
    std::unique_ptr<printing::PrinterQuery> printer_query = m_printerQueriesQueue->PopPrinterQuery(cookie);
    if (!printer_query)
         printer_query = m_printerQueriesQueue->CreatePrinterQuery(content::GlobalRenderFrameHostId());

    auto *printer_query_ptr = printer_query.get();
    printer_query_ptr->SetSettings(
                    std::move(job_settings),
                    base::BindOnce(&OnDidUpdatePrintSettings,
                                   m_printerQueriesQueue, std::move(printer_query), std::move(callback),
                                   render_frame_host->GetProcess()->GetID(), render_frame_host->GetRoutingID()));
}

} // namespace QtWebEngineCore
