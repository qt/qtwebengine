/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "print_view_manager_qt.h"

#include "type_conversion.h"
#include "web_engine_context.h"

#include <QtGui/QPageLayout>
#include <QtGui/QPageSize>

#include "base/values.h"
#include "chrome/browser/printing/print_job_manager.h"
#include "chrome/browser/printing/printer_query.h"
#include "components/printing/common/print_messages.h"
#include "content/public/browser/browser_thread.h"
#include "printing/pdf_metafile_skia.h"
#include "printing/print_job_constants.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(QtWebEngineCore::PrintViewManagerQt);

namespace {
static int request_id = 0;
static const qreal kMicronsToMillimeter = 1000.0f;

static scoped_refptr<base::RefCountedBytes>
GetDataFromHandle(base::SharedMemoryHandle handle, uint32 data_size) {
    scoped_ptr<base::SharedMemory> shared_buf(
                new base::SharedMemory(handle, true));

    if (!shared_buf->Map(data_size)) {
        NOTREACHED();
        return NULL;
    }

    unsigned char* data = static_cast<unsigned char*>(shared_buf->memory());
    std::vector<unsigned char> dataVector(data, data + data_size);
    return base::RefCountedBytes::TakeVector(&dataVector);
}

// Write the PDF file to disk.
static void SavePdfFile(scoped_refptr<base::RefCountedBytes> data,
                        const base::FilePath& path,
                        const base::Callback<void(bool)>& callback) {
    DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);
    DCHECK_GT(data->size(), 0U);

    printing::PdfMetafileSkia metafile;
    metafile.InitFromData(static_cast<const void*>(data->front()), data->size());

    base::File file(path,
                    base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    bool ok = file.IsValid() && metafile.SaveTo(&file);

    if (!callback.is_null()) {
        content::BrowserThread::PostTask(content::BrowserThread::UI,
                                         FROM_HERE,
                                         base::Bind(callback, ok));
    }
}

static void applyQPageLayoutSettingsToDictionary(const QPageLayout& pageLayout, base::DictionaryValue& print_settings)
{
    //Set page size attributes, chromium expects these in micrometers
    QSizeF pageSizeInMilimeter = pageLayout.pageSize().size(QPageSize::Millimeter);
    scoped_ptr<base::DictionaryValue> sizeDict(new base::DictionaryValue);
    sizeDict->SetInteger(printing::kSettingMediaSizeWidthMicrons, pageSizeInMilimeter.width() * kMicronsToMillimeter);
    sizeDict->SetInteger(printing::kSettingMediaSizeHeightMicrons, pageSizeInMilimeter.height() * kMicronsToMillimeter);
    print_settings.Set(printing::kSettingMediaSize, sizeDict.Pass());

    print_settings.SetBoolean(printing::kSettingLandscape, pageLayout.orientation() == QPageLayout::Landscape);

    print_settings.SetInteger(printing::kPreviewRequestID, request_id++);
    print_settings.SetBoolean(printing::kIsFirstRequest, request_id != 0);

    // The following are standard settings that Chromium expects to be set.
    print_settings.SetBoolean(printing::kSettingPrintToPDF, true);
    print_settings.SetBoolean(printing::kSettingCloudPrintDialog, false);
    print_settings.SetBoolean(printing::kSettingPrintWithPrivet, false);
    print_settings.SetBoolean(printing::kSettingPrintWithExtension, false);

    print_settings.SetBoolean(printing::kSettingGenerateDraftData, false);
    print_settings.SetBoolean(printing::kSettingPreviewModifiable, false);
    print_settings.SetInteger(printing::kSettingColor, printing::COLOR);
    print_settings.SetInteger(printing::kSettingDuplexMode, printing::SIMPLEX);
    print_settings.SetInteger(printing::kSettingDuplexMode, printing::UNKNOWN_DUPLEX_MODE);
    print_settings.SetInteger(printing::kSettingCopies, 1);
    print_settings.SetBoolean(printing::kSettingCollate, false);
    print_settings.SetBoolean(printing::kSettingGenerateDraftData, false);
    print_settings.SetBoolean(printing::kSettingPreviewModifiable, false);

    print_settings.SetBoolean(printing::kSettingShouldPrintSelectionOnly, false);
    print_settings.SetBoolean(printing::kSettingShouldPrintBackgrounds, false);
    print_settings.SetBoolean(printing::kSettingHeaderFooterEnabled, false);
    print_settings.SetString(printing::kSettingDeviceName, "");
    print_settings.SetInteger(printing::kPreviewUIID, 12345678);
}

} // namespace

namespace QtWebEngineCore {

PrintViewManagerQt::~PrintViewManagerQt()
{
}

#if defined(ENABLE_BASIC_PRINTING)
bool PrintViewManagerQt::PrintToPDF(const QPageLayout &pageLayout, const QString &filePath)
{
    return PrintToPDFWithCallback(pageLayout, filePath, base::Callback<void(bool)>());
}

bool PrintViewManagerQt::PrintToPDFWithCallback(const QPageLayout &pageLayout, const QString &filePath, base::Callback<void(bool)> callback)
{
    // If there already is a pending print in progress, don't try starting another one.
    if (m_printSettings)
        return false;

    m_printSettings.reset(new base::DictionaryValue());
    applyQPageLayoutSettingsToDictionary(pageLayout, *m_printSettings);
    m_pdfOutputPath = toFilePath(filePath);

    return Send(new PrintMsg_InitiatePrintPreview(routing_id(), false));
}
#endif // defined(ENABLE_BASIC_PRINTING)

// PrintedPagesSource implementation.
base::string16 PrintViewManagerQt::RenderSourceName()
{
     return toString16(QLatin1String(""));
}

PrintViewManagerQt::PrintViewManagerQt(content::WebContents *contents)
    : PrintViewManagerBaseQt(contents)
{

}

// content::WebContentsObserver implementation.
bool PrintViewManagerQt::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(PrintViewManagerQt, message)
      IPC_MESSAGE_HANDLER(PrintHostMsg_DidShowPrintDialog, OnDidShowPrintDialog)
      IPC_MESSAGE_HANDLER(PrintHostMsg_RequestPrintPreview,
                                 OnRequestPrintPreview)
      IPC_MESSAGE_HANDLER(PrintHostMsg_MetafileReadyForPrinting,
                                 OnMetafileReadyForPrinting);
      IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled || PrintManager::OnMessageReceived(message);
}

void PrintViewManagerQt::resetPdfState()
{
    m_pdfOutputPath.clear();
    m_pdfPrintCallback.Reset();
    m_printSettings.reset();
}

// IPC handlers

void PrintViewManagerQt::OnRequestPrintPreview(
    const PrintHostMsg_RequestPrintPreview_Params& params)
{
    Send(new PrintMsg_PrintPreview(routing_id(), *m_printSettings));
}

void PrintViewManagerQt::OnMetafileReadyForPrinting(
    const PrintHostMsg_DidPreviewDocument_Params& params)
{
    StopWorker(params.document_cookie);

    scoped_refptr<base::RefCountedBytes> data_bytes =
        GetDataFromHandle(params.metafile_data_handle, params.data_size);
    if (!data_bytes || !data_bytes->size()) {
      resetPdfState();
      return;
    }

    // Create local copies so we can reset the state and take a new pdf print job.
    base::Callback<void(bool)> pdf_print_callback = m_pdfPrintCallback;
    base::FilePath pdfOutputPath = m_pdfOutputPath;

    // Save the PDF file to disk and then execute the callback.
    content::BrowserThread::PostTask(content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&SavePdfFile, data_bytes, pdfOutputPath,
                   pdf_print_callback));

    resetPdfState();
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
    resetPdfState();
}

void PrintViewManagerQt::RenderProcessGone(base::TerminationStatus status)
{
    PrintViewManagerBaseQt::RenderProcessGone(status);
    resetPdfState();
}


} // namespace QtWebEngineCore
