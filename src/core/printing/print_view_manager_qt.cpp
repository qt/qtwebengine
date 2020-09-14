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

#include "print_view_manager_qt.h"

#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

#include <QtGui/qpagelayout.h>
#include <QtGui/qpagesize.h>

#include "base/values.h"
#include "base/memory/ref_counted_memory.h"
#include "base/task/post_task.h"
#include "chrome/browser/printing/print_job_manager.h"
#include "chrome/browser/printing/printer_query.h"
#include "components/printing/common/print_messages.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/common/web_preferences.h"
#include "printing/metafile_skia.h"
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
                        const QtWebEngineCore::PrintViewManagerQt::PrintToPDFFileCallback &saveCallback)
{
    DCHECK_GT(data->size(), 0U);

    printing::MetafileSkia metafile;
    metafile.InitFromData(base::as_bytes(base::make_span(data->front(), data->size())));

    base::File file(path,
                    base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    bool success = file.IsValid() && metafile.SaveTo(&file);
    base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                   base::BindOnce(saveCallback, success));
}

static base::DictionaryValue *createPrintSettings()
{
    base::DictionaryValue *printSettings = new base::DictionaryValue();
    // TO DO: Check if we can use the request ID from Qt here somehow.
    static int internalRequestId = 0;

    printSettings->SetBoolean(printing::kIsFirstRequest, internalRequestId++ == 0);
    printSettings->SetInteger(printing::kPreviewRequestID, internalRequestId);

    // The following are standard settings that Chromium expects to be set.
    printSettings->SetInteger(printing::kSettingPrinterType, printing::kPdfPrinter);

    printSettings->SetInteger(printing::kSettingDpiHorizontal, printing::kPointsPerInch);
    printSettings->SetInteger(printing::kSettingDpiVertical, printing::kPointsPerInch);

    printSettings->SetInteger(printing::kSettingDuplexMode, printing::SIMPLEX);
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
        printSettings->SetInteger(printing::kSettingMarginsType, printing::CUSTOM_MARGINS);

        // pageSizeInMillimeter is in portrait orientation. Transpose it if necessary.
        printSettings->SetBoolean(printing::kSettingLandscape, pageLayout.orientation() == QPageLayout::Landscape);
    } else {
        // QPrinter will handle margins
        pageSizeInMillimeter = pageLayout.paintRect(QPageLayout::Millimeter);
        printSettings->SetInteger(printing::kSettingMarginsType, printing::NO_MARGINS);

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

} // namespace

namespace QtWebEngineCore {

struct PrintViewManagerQt::FrameDispatchHelper {
    PrintViewManagerQt* m_manager;
    content::RenderFrameHost* m_renderFrameHost;

    bool Send(IPC::Message* msg) {
        return m_renderFrameHost->Send(msg);
    }

    void OnSetupScriptedPrintPreview(IPC::Message* reply_msg) {
        m_manager->OnSetupScriptedPrintPreview(m_renderFrameHost, reply_msg);
    }
};

PrintViewManagerQt::~PrintViewManagerQt()
{
}

void PrintViewManagerQt::PrintToPDFFileWithCallback(const QPageLayout &pageLayout,
                                                    bool printInColor,
                                                    const QString &filePath,
                                                    const PrintToPDFFileCallback& callback)
{
    if (callback.is_null())
        return;

    if (m_printSettings || !filePath.length()) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(callback, false));
        return;
    }

    m_pdfOutputPath = toFilePath(filePath);
    m_pdfSaveCallback = callback;
    if (!PrintToPDFInternal(pageLayout, printInColor)) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(callback, false));
        resetPdfState();
    }
}

void PrintViewManagerQt::PrintToPDFWithCallback(const QPageLayout &pageLayout,
                                                bool printInColor,
                                                bool useCustomMargins,
                                                const PrintToPDFCallback& callback)
{
    if (callback.is_null())
        return;

    // If there already is a pending print in progress, don't try starting another one.
    if (m_printSettings) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(callback, QSharedPointer<QByteArray>()));
        return;
    }

    m_pdfPrintCallback = callback;
    if (!PrintToPDFInternal(pageLayout, printInColor, useCustomMargins)) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(callback, QSharedPointer<QByteArray>()));

        resetPdfState();
    }
}

bool PrintViewManagerQt::PrintToPDFInternal(const QPageLayout &pageLayout,
                                            const bool printInColor,
                                            const bool useCustomMargins)
{
    if (!pageLayout.isValid())
        return false;

    m_printSettings.reset(createPrintSettingsFromQPageLayout(pageLayout, useCustomMargins));
    m_printSettings->SetBoolean(printing::kSettingShouldPrintBackgrounds,
                                web_contents()->GetRenderViewHost()->
                                GetWebkitPreferences().should_print_backgrounds);
    m_printSettings->SetInteger(printing::kSettingColor,
                                printInColor ? printing::COLOR : printing::GRAYSCALE);

    if (web_contents()->ShowingInterstitialPage() || web_contents()->IsCrashed())
        return false;

    content::RenderFrameHost* rfh = web_contents()->GetMainFrame();
    GetPrintRenderFrame(rfh)->InitiatePrintPreview(mojo::PendingAssociatedRemote<printing::mojom::PrintRenderer>(), false);

    DCHECK(!m_printPreviewRfh);
    m_printPreviewRfh = rfh;
    return true;
}

// PrintedPagesSource implementation.
base::string16 PrintViewManagerQt::RenderSourceName()
{
    return base::string16();
}

PrintViewManagerQt::PrintViewManagerQt(content::WebContents *contents)
    : PrintViewManagerBaseQt(contents)
    , m_printPreviewRfh(nullptr)
{

}

// content::WebContentsObserver implementation.
bool PrintViewManagerQt::OnMessageReceived(const IPC::Message& message,
                                           content::RenderFrameHost* render_frame_host)
{
    FrameDispatchHelper helper = {this, render_frame_host};
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP_WITH_PARAM(PrintViewManagerQt, message, render_frame_host);
        IPC_MESSAGE_HANDLER(PrintHostMsg_DidShowPrintDialog, OnDidShowPrintDialog)
        IPC_MESSAGE_HANDLER(PrintHostMsg_RequestPrintPreview, OnRequestPrintPreview)
        IPC_MESSAGE_HANDLER(PrintHostMsg_MetafileReadyForPrinting, OnMetafileReadyForPrinting);
        IPC_MESSAGE_HANDLER(PrintHostMsg_DidPreviewPage, OnDidPreviewPage)
        IPC_MESSAGE_FORWARD_DELAY_REPLY(
                PrintHostMsg_SetupScriptedPrintPreview, &helper,
                FrameDispatchHelper::OnSetupScriptedPrintPreview)
        IPC_MESSAGE_HANDLER(PrintHostMsg_ShowScriptedPrintPreview,
                                       OnShowScriptedPrintPreview)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled || PrintViewManagerBaseQt::OnMessageReceived(message, render_frame_host);
}

void PrintViewManagerQt::RenderFrameDeleted(content::RenderFrameHost *render_frame_host)
{
    if (render_frame_host == m_printPreviewRfh)
        PrintPreviewDone();
    PrintViewManagerBaseQt::RenderFrameDeleted(render_frame_host);
    m_printRenderFrames.erase(render_frame_host);
}

const mojo::AssociatedRemote<printing::mojom::PrintRenderFrame> &PrintViewManagerQt::GetPrintRenderFrame(content::RenderFrameHost *rfh)
{
    auto it = m_printRenderFrames.find(rfh);
    if (it == m_printRenderFrames.end()) {
        mojo::AssociatedRemote<printing::mojom::PrintRenderFrame> remote;
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(&remote);
        it = m_printRenderFrames.insert(std::make_pair(rfh, std::move(remote))).first;
    } else if (it->second.is_bound() && !it->second.is_connected()) {
        // When print preview is closed, the remote is disconnected from the
        // receiver. Reset and bind the remote before using it again.
        it->second.reset();
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(&it->second);
    }

    return it->second;
}

void PrintViewManagerQt::resetPdfState()
{
    m_pdfOutputPath.clear();
    m_pdfPrintCallback.Reset();
    m_pdfSaveCallback.Reset();
    m_printSettings.reset();
}

// IPC handlers

void PrintViewManagerQt::OnRequestPrintPreview(
    const PrintHostMsg_RequestPrintPreview_Params & /*params*/)
{
    mojo::AssociatedRemote<printing::mojom::PrintRenderFrame> printRenderFrame;
    m_printPreviewRfh->GetRemoteAssociatedInterfaces()->GetInterface(&printRenderFrame);
    printRenderFrame->PrintPreview(m_printSettings->Clone());
    PrintPreviewDone();
}

void PrintViewManagerQt::OnMetafileReadyForPrinting(content::RenderFrameHost* rfh,
                                                    const PrintHostMsg_DidPreviewDocument_Params& params,
                                                    const PrintHostMsg_PreviewIds &ids)
{
    StopWorker(params.document_cookie);

    // Create local copies so we can reset the state and take a new pdf print job.
    PrintToPDFCallback pdf_print_callback = std::move(m_pdfPrintCallback);
    PrintToPDFFileCallback pdf_save_callback = std::move(m_pdfSaveCallback);
    base::FilePath pdfOutputPath = m_pdfOutputPath;

    resetPdfState();

    if (!pdf_print_callback.is_null()) {
        QSharedPointer<QByteArray> data_array = GetStdVectorFromHandle(params.content.metafile_data_region);
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(pdf_print_callback, data_array));
    } else {
        scoped_refptr<base::RefCountedBytes> data_bytes = GetBytesFromHandle(params.content.metafile_data_region);
        base::PostTask(FROM_HERE, {base::ThreadPool(), base::MayBlock()},
                       base::BindOnce(&SavePdfFile, data_bytes, pdfOutputPath, pdf_save_callback));
    }
}

void PrintViewManagerQt::OnDidShowPrintDialog()
{
}

// content::WebContentsObserver implementation.
void PrintViewManagerQt::DidStartLoading()
{
}

// content::WebContentsObserver implementation.
// Cancels the print job.
void PrintViewManagerQt::NavigationStopped()
{
    if (!m_pdfPrintCallback.is_null()) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(m_pdfPrintCallback, QSharedPointer<QByteArray>()));
    }
    resetPdfState();
    PrintViewManagerBaseQt::NavigationStopped();
}

void PrintViewManagerQt::RenderProcessGone(base::TerminationStatus status)
{
    PrintViewManagerBaseQt::RenderProcessGone(status);
    if (!m_pdfPrintCallback.is_null()) {
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(m_pdfPrintCallback, QSharedPointer<QByteArray>()));
    }
    resetPdfState();
}

void PrintViewManagerQt::OnDidPreviewPage(content::RenderFrameHost* rfh,
                                          const PrintHostMsg_DidPreviewPage_Params& params,
                                          const PrintHostMsg_PreviewIds& ids)
{
    // just consume the message, this is just for sending 'page-preview-ready' for webui
}

void PrintViewManagerQt::OnSetupScriptedPrintPreview(content::RenderFrameHost* rfh,
                                                     IPC::Message* reply_msg)
{
    // ignore the scripted print
    rfh->Send(reply_msg);

    content::WebContentsView *view = static_cast<content::WebContentsImpl*>(web_contents())->GetView();
    WebContentsAdapterClient *client = WebContentsViewQt::from(view)->client();

    if (!client)
        return;

    // close preview
    GetPrintRenderFrame(rfh)->OnPrintPreviewDialogClosed();

    client->printRequested();
}

void PrintViewManagerQt::OnShowScriptedPrintPreview(content::RenderFrameHost* rfh,
                                                    bool source_is_modifiable)
{
    // ignore for now
}

void PrintViewManagerQt::PrintPreviewDone() {
    GetPrintRenderFrame(m_printPreviewRfh)->OnPrintPreviewDialogClosed();
    m_printPreviewRfh = nullptr;
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(PrintViewManagerQt)

} // namespace QtWebEngineCore
