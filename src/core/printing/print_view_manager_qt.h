// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#ifndef PRINT_VIEW_MANAGER_QT_H
#define PRINT_VIEW_MANAGER_QT_H

#include "print_view_manager_base_qt.h"

#include "qtwebenginecoreglobal_p.h"

#include "base/memory/ref_counted.h"
#include "components/prefs/pref_member.h"
#include "components/printing/common/print.mojom.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/associated_remote.h"

#include <QSharedPointer>

QT_BEGIN_NAMESPACE
class QPageLayout;
class QPageRanges;
class QString;
QT_END_NAMESPACE

namespace QtWebEngineCore {
class PrintViewManagerQt
        : public PrintViewManagerBaseQt
        , public content::WebContentsUserData<PrintViewManagerQt>
{
public:
    ~PrintViewManagerQt() override;

    static void BindPrintManagerHost(mojo::PendingAssociatedReceiver<printing::mojom::PrintManagerHost> receiver,
                                     content::RenderFrameHost *rfh);

    typedef base::OnceCallback<void(QSharedPointer<QByteArray> result)> PrintToPDFCallback;
    typedef base::OnceCallback<void(bool success)> PrintToPDFFileCallback;

    // Method to print a page to a Pdf document with page size \a pageSize in location \a filePath.
    void PrintToPDFFileWithCallback(const QPageLayout &pageLayout,
                                    const QPageRanges &pageRanges,
                                    bool printInColor,
                                    const QString &filePath,
                                    PrintToPDFFileCallback callback);
    void PrintToPDFWithCallback(const QPageLayout &pageLayout,
                                const QPageRanges &pageRanges,
                                bool printInColor,
                                bool useCustomMargins,
                                PrintToPDFCallback callback);

protected:
    explicit PrintViewManagerQt(content::WebContents*);

    bool PrintToPDFInternal(const QPageLayout &, const QPageRanges &, bool printInColor, bool useCustomMargins = true);

    // content::WebContentsObserver implementation.
    // Cancels the print job.
    void NavigationStopped() override;

    // Terminates or cancels the print job if one was pending.
    void PrimaryMainFrameRenderProcessGone(base::TerminationStatus status) override;

    void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;

    // mojom::PrintManagerHost:
    void SetupScriptedPrintPreview(SetupScriptedPrintPreviewCallback callback) override;
    void ShowScriptedPrintPreview(bool source_is_modifiable) override;
    void RequestPrintPreview(printing::mojom::RequestPrintPreviewParamsPtr params) override;
    void CheckForCancel(int32_t preview_ui_id,
                        int32_t request_id,
                        CheckForCancelCallback callback) override;
    void MetafileReadyForPrinting(printing::mojom::DidPreviewDocumentParamsPtr params,
                                  int32_t preview_ui_id) override;
    void SetAccessibilityTree(int32_t, const ui::AXTreeUpdate &) override;
private:
    void resetPdfState();

    void PrintPreviewDone();

private:
    WEB_CONTENTS_USER_DATA_KEY_DECL();
    content::RenderFrameHost *m_printPreviewRfh;
    base::FilePath m_pdfOutputPath;
    PrintToPDFCallback m_pdfPrintCallback;
    PrintToPDFFileCallback m_pdfSaveCallback;
    base::Value::Dict m_printSettings;

    friend class content::WebContentsUserData<PrintViewManagerQt>;
};

} // namespace QtWebEngineCore
#endif // PRINT_VIEW_MANAGER_QT_H

