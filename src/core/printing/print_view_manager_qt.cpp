// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Loosely based on print_view_manager.cc and print_preview_message_handler.cc
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "print_view_manager_qt.h"

#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

#include <QtGui/qpagelayout.h>
#include <QtGui/qpageranges.h>
#include <QtGui/qpagesize.h>

#include "base/values.h"
#include "base/memory/ref_counted_memory.h"
#include "base/task/post_task.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/printing/print_job_manager.h"
#include "chrome/browser/printing/printer_query.h"
#include "components/printing/common/print.mojom.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "printing/metafile_skia.h"
#include "printing/mojom/print.mojom-shared.h"
#include "printing/print_job_constants.h"
#include "printing/units.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace {

static const qreal kMicronsToMillimeter = 1000.0f;

static QSharedPointer<QByteArray> GetStdVectorFromHandle(const base::ReadOnlySharedMemoryRegion &handle)
{
    base::ReadOnlySharedMemoryMapping map = handle.Map();
    if (!map.IsValid())
        return QSharedPointer<QByteArray>(new QByteArray);

    const char* data = static_cast<const char*>(map.memory());
    return QSharedPointer<QByteArray>(new QByteArray(data, map.size()));
}

static scoped_refptr<base::RefCountedBytes>
GetBytesFromHandle(const base::ReadOnlySharedMemoryRegion &handle)
{
    base::ReadOnlySharedMemoryMapping map = handle.Map();
    if (!map.IsValid())
        return nullptr;

    const unsigned char* data = static_cast<const unsigned char*>(map.memory());
    std::vector<unsigned char> dataVector(data, data + map.size());
    return base::RefCountedBytes::TakeVector(&dataVector);
}

// Write the PDF file to disk.
static void SavePdfFile(scoped_refptr<base::RefCountedBytes> data,
                        const base::FilePath &path,
                        QtWebEngineCore::PrintViewManagerQt::PrintToPDFFileCallback saveCallback)
{
    DCHECK_GT(data->size(), 0U);

    printing::MetafileSkia metafile;
    metafile.InitFromData(base::as_bytes(base::make_span(data->front(), data->size())));

    base::File file(path,
                    base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    bool success = file.IsValid() && metafile.SaveTo(&file);
    base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                   base::BindOnce(std::move(saveCallback), success));
}

static base::DictionaryValue *createPrintSettings()
{
    base::DictionaryValue *printSettings = new base::DictionaryValue();
    // TO DO: Check if we can use the request ID from Qt here somehow.
    static int internalRequestId = 0;

    printSettings->SetBoolean(printing::kIsFirstRequest, internalRequestId++ == 0);
    printSettings->SetInteger(printing::kPreviewRequestID, internalRequestId);

    // The following are standard settings that Chromium expects to be set.
    printSettings->SetInteger(printing::kSettingPrinterType, static_cast<int>(printing::mojom::PrinterType::kPdf));

    printSettings->SetInteger(printing::kSettingDpiHorizontal, printing::kPointsPerInch);
    printSettings->SetInteger(printing::kSettingDpiVertical, printing::kPointsPerInch);

    printSettings->SetInteger(printing::kSettingDuplexMode, static_cast<int>(printing::mojom::DuplexMode::kSimplex));
    printSettings->SetInteger(printing::kSettingCopies, 1);
    printSettings->SetInteger(printing::kSettingPagesPerSheet, 1);
    printSettings->SetBoolean(printing::kSettingCollate, false);
//    printSettings->SetBoolean(printing::kSettingGenerateDraftData, false);
    printSettings->SetBoolean(printing::kSettingPreviewModifiable, false);

    printSettings->SetKey(printing::kSettingShouldPrintSelectionOnly, base::Value(false));
    printSettings->SetKey(printing::kSettingShouldPrintBackgrounds, base::Value(true));
    printSettings->SetKey(printing::kSettingHeaderFooterEnabled, base::Value(false));
    printSettings->SetKey(printing::kSettingRasterizePdf, base::Value(false));
    printSettings->SetInteger(printing::kSettingScaleFactor, 100);
    printSettings->SetString(printing::kSettingDeviceName, "");
    printSettings->SetInteger(printing::kPreviewUIID, 12345678);

    return printSettings;
}

static base::DictionaryValue *createPrintSettingsFromQPageLayout(const QPageLayout &pageLayout,
                                                                 bool useCustomMargins)
{
    base::DictionaryValue *printSettings = createPrintSettings();
    QRectF pageSizeInMillimeter;

    if (useCustomMargins) {
        // Apply page margins when printing to PDF
        pageSizeInMillimeter = pageLayout.pageSize().rect(QPageSize::Millimeter);

        QMargins pageMarginsInPoints = pageLayout.marginsPoints();
        std::unique_ptr<base::DictionaryValue> marginsDict(new base::DictionaryValue);
        marginsDict->SetInteger(printing::kSettingMarginTop, pageMarginsInPoints.top());
        marginsDict->SetInteger(printing::kSettingMarginBottom, pageMarginsInPoints.bottom());
        marginsDict->SetInteger(printing::kSettingMarginLeft, pageMarginsInPoints.left());
        marginsDict->SetInteger(printing::kSettingMarginRight, pageMarginsInPoints.right());

        printSettings->Set(printing::kSettingMarginsCustom, std::move(marginsDict));
        printSettings->SetInteger(printing::kSettingMarginsType, (int)printing::mojom::MarginType::kCustomMargins);

        // pageSizeInMillimeter is in portrait orientation. Transpose it if necessary.
        printSettings->SetBoolean(printing::kSettingLandscape, pageLayout.orientation() == QPageLayout::Landscape);
    } else {
        // QPrinter will handle margins
        pageSizeInMillimeter = pageLayout.paintRect(QPageLayout::Millimeter);
        printSettings->SetInteger(printing::kSettingMarginsType, (int)printing::mojom::MarginType::kNoMargins);

        // pageSizeInMillimeter already contains the orientation.
        printSettings->SetBoolean(printing::kSettingLandscape, false);
    }

    //Set page size attributes, chromium expects these in micrometers
    std::unique_ptr<base::DictionaryValue> sizeDict(new base::DictionaryValue);
    sizeDict->SetInteger(printing::kSettingMediaSizeWidthMicrons, pageSizeInMillimeter.width() * kMicronsToMillimeter);
    sizeDict->SetInteger(printing::kSettingMediaSizeHeightMicrons, pageSizeInMillimeter.height() * kMicronsToMillimeter);
    printSettings->Set(printing::kSettingMediaSize, std::move(sizeDict));

    return printSettings;
}

static base::ListValue *createPageRangeSettings(const QList<QPageRanges::Range> &ranges)
{
    base::ListValue *pageRangeArray = new base::ListValue;
    for (int i = 0; i < ranges.count(); i++) {
        std::unique_ptr<base::DictionaryValue> pageRange(new base::DictionaryValue);
        pageRange->SetInteger(printing::kSettingPageRangeFrom, ranges.at(i).from);
        pageRange->SetInteger(printing::kSettingPageRangeTo, ranges.at(i).to);
        pageRangeArray->Append(std::move(pageRange));
    }
    return pageRangeArray;
}

} // namespace

namespace QtWebEngineCore {

PrintViewManagerQt::~PrintViewManagerQt()
{
}

void PrintViewManagerQt::PrintToPDFFileWithCallback(const QPageLayout &pageLayout,
                                                    const QPageRanges &pageRanges,
                                                    bool printInColor,
                                                    const QString &filePath,
                                                    PrintToPDFFileCallback callback)
{
    if (callback.is_null())
        return;

    if (m_printSettings || !filePath.length()) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(callback), false));
        return;
    }

    m_pdfOutputPath = toFilePath(filePath);
    m_pdfSaveCallback = std::move(callback);
    if (!PrintToPDFInternal(pageLayout, pageRanges, printInColor)) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(m_pdfSaveCallback), false));
        resetPdfState();
    }
}

void PrintViewManagerQt::PrintToPDFWithCallback(const QPageLayout &pageLayout,
                                                const QPageRanges &pageRanges,
                                                bool printInColor,
                                                bool useCustomMargins,
                                                PrintToPDFCallback callback)
{
    if (callback.is_null())
        return;

    // If there already is a pending print in progress, don't try starting another one.
    if (m_printSettings) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(callback), QSharedPointer<QByteArray>()));
        return;
    }

    m_pdfPrintCallback = std::move(callback);
    if (!PrintToPDFInternal(pageLayout, pageRanges, printInColor, useCustomMargins)) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(m_pdfPrintCallback), QSharedPointer<QByteArray>()));

        resetPdfState();
    }
}

bool PrintViewManagerQt::PrintToPDFInternal(const QPageLayout &pageLayout,
                                            const QPageRanges &pageRanges,
                                            const bool printInColor,
                                            const bool useCustomMargins)
{
    if (!pageLayout.isValid())
        return false;

    m_printSettings.reset(createPrintSettingsFromQPageLayout(pageLayout, useCustomMargins));
    m_printSettings->SetBoolean(printing::kSettingShouldPrintBackgrounds,
                                web_contents()->GetOrCreateWebPreferences().should_print_backgrounds);
    m_printSettings->SetInteger(printing::kSettingColor,
                                int(printInColor ? printing::mojom::ColorModel::kColor : printing::mojom::ColorModel::kGrayscale));
    if (!pageRanges.isEmpty())
        m_printSettings->Set(printing::kSettingPageRange,
                             std::unique_ptr<base::ListValue>(createPageRangeSettings(pageRanges.toRangeList())));

    if (web_contents()->IsCrashed())
        return false;

    content::RenderFrameHost* rfh = web_contents()->GetMainFrame();
    GetPrintRenderFrame(rfh)->InitiatePrintPreview(mojo::PendingAssociatedRemote<printing::mojom::PrintRenderer>(), false);

    DCHECK(!m_printPreviewRfh);
    m_printPreviewRfh = rfh;
    return true;
}

PrintViewManagerQt::PrintViewManagerQt(content::WebContents *contents)
    : PrintViewManagerBaseQt(contents)
    , content::WebContentsUserData<PrintViewManagerQt>(*contents)
    , m_printPreviewRfh(nullptr)
{

}

// static
void PrintViewManagerQt::BindPrintManagerHost(mojo::PendingAssociatedReceiver<printing::mojom::PrintManagerHost> receiver,
                                              content::RenderFrameHost *rfh)
{
    auto *web_contents = content::WebContents::FromRenderFrameHost(rfh);
    if (!web_contents)
        return;
    auto *print_manager = PrintViewManagerQt::FromWebContents(web_contents);
    if (!print_manager)
        return;
    print_manager->BindReceiver(std::move(receiver), rfh);
}

void PrintViewManagerQt::resetPdfState()
{
    m_pdfOutputPath.clear();
    m_pdfPrintCallback.Reset();
    m_pdfSaveCallback.Reset();
    m_printSettings.reset();
}

void PrintViewManagerQt::PrintPreviewDone()
{
    if (IsPrintRenderFrameConnected(m_printPreviewRfh))
        GetPrintRenderFrame(m_printPreviewRfh)->OnPrintPreviewDialogClosed();
    m_printPreviewRfh = nullptr;
}

// content::WebContentsObserver implementation.
// Cancels the print job.
void PrintViewManagerQt::NavigationStopped()
{
    if (!m_pdfPrintCallback.is_null()) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(m_pdfPrintCallback), QSharedPointer<QByteArray>()));
    }
    resetPdfState();
    PrintViewManagerBaseQt::NavigationStopped();
}

void PrintViewManagerQt::PrimaryMainFrameRenderProcessGone(base::TerminationStatus status)
{
    PrintViewManagerBaseQt::PrimaryMainFrameRenderProcessGone(status);
    if (!m_pdfPrintCallback.is_null()) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(m_pdfPrintCallback), QSharedPointer<QByteArray>()));
    }
    resetPdfState();
}

void PrintViewManagerQt::RenderFrameDeleted(content::RenderFrameHost *render_frame_host)
{
    if (render_frame_host == m_printPreviewRfh)
        PrintPreviewDone();
    PrintViewManagerBaseQt::RenderFrameDeleted(render_frame_host);
}

// mojom::PrintManagerHost:
void PrintViewManagerQt::SetupScriptedPrintPreview(SetupScriptedPrintPreviewCallback callback)
{
    // ignore the scripted print
    std::move(callback).Run();

    content::WebContentsView *view = static_cast<content::WebContentsImpl*>(web_contents())->GetView();
    WebContentsAdapterClient *client = WebContentsViewQt::from(view)->client();
    content::RenderFrameHost *rfh =
        print_manager_host_receivers_.GetCurrentTargetFrame();
    if (!client)
        return;

    // close preview
    if (rfh)
        GetPrintRenderFrame(rfh)->OnPrintPreviewDialogClosed();

    client->printRequested();
}

void PrintViewManagerQt::ShowScriptedPrintPreview(bool /*source_is_modifiable*/)
{
    // ignore for now
}

void PrintViewManagerQt::RequestPrintPreview(printing::mojom::RequestPrintPreviewParamsPtr /*params*/)
{
    mojo::AssociatedRemote<printing::mojom::PrintRenderFrame> printRenderFrame;
    m_printPreviewRfh->GetRemoteAssociatedInterfaces()->GetInterface(&printRenderFrame);
    printRenderFrame->PrintPreview(m_printSettings->Clone());
    PrintPreviewDone();
}

void PrintViewManagerQt::CheckForCancel(int32_t preview_ui_id,
                                        int32_t request_id,
                                        CheckForCancelCallback callback)
{
    Q_UNUSED(preview_ui_id);
    Q_UNUSED(request_id);
    std::move(callback).Run(false);
}

void PrintViewManagerQt::SetAccessibilityTree(int32_t, const ui::AXTreeUpdate &)
{
    // FIXME!
}

void PrintViewManagerQt::MetafileReadyForPrinting(printing::mojom::DidPreviewDocumentParamsPtr params,
                                                  int32_t preview_ui_id)
{
    Q_UNUSED(preview_ui_id);
    StopWorker(params->document_cookie);

    // Create local copies so we can reset the state and take a new pdf print job.
    PrintToPDFCallback pdf_print_callback = std::move(m_pdfPrintCallback);
    PrintToPDFFileCallback pdf_save_callback = std::move(m_pdfSaveCallback);
    base::FilePath pdfOutputPath = m_pdfOutputPath;

    resetPdfState();

    if (!pdf_print_callback.is_null()) {
        QSharedPointer<QByteArray> data_array = GetStdVectorFromHandle(params->content->metafile_data_region);
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(std::move(pdf_print_callback), data_array));
    } else {
        scoped_refptr<base::RefCountedBytes> data_bytes = GetBytesFromHandle(params->content->metafile_data_region);
        base::ThreadPool::PostTask(FROM_HERE, { base::MayBlock() },
                                   base::BindOnce(&SavePdfFile, data_bytes, pdfOutputPath, std::move(pdf_save_callback)));
    }
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(PrintViewManagerQt);

} // namespace QtWebEngineCore
