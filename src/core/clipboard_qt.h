// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CLIPBOARD_QT_H
#define CLIPBOARD_QT_H

#include "ui/base/clipboard/clipboard.h"

namespace QtWebEngineCore {

class ClipboardQt : public ui::Clipboard
{
public:
    const ui::ClipboardSequenceNumberToken &GetSequenceNumber(ui::ClipboardBuffer type) const override;
    bool IsFormatAvailable(const ui::ClipboardFormatType &format,
                           ui::ClipboardBuffer buffer,
                           const ui::DataTransferEndpoint *data_dst) const override;
    void Clear(ui::ClipboardBuffer type) override;
    void ReadAvailableTypes(ui::ClipboardBuffer type,
                            const ui::DataTransferEndpoint *data_dst,
                            std::vector<std::u16string> *types) const override;
    void ReadText(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::u16string *result) const override;
    void ReadAsciiText(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::string *result) const override;
    void ReadHTML(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::u16string *markup, std::string *src_url, uint32_t *fragment_start,
                  uint32_t *fragment_end) const override;
    void ReadRTF(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::string *result) const override;
    void ReadCustomData(ui::ClipboardBuffer clipboard_type, const std::u16string &type, const ui::DataTransferEndpoint *data_dst, std::u16string *result) const override;
    void ReadBookmark(const ui::DataTransferEndpoint *data_dst, std::u16string *title, std::string *url) const override;
    void ReadData(const ui::ClipboardFormatType &format, const ui::DataTransferEndpoint *data_dst, std::string *result) const override;
#if defined(USE_OZONE)
    bool IsSelectionBufferAvailable() const override;
#endif
    void OnPreShutdown() override {}
    void ReadSvg(ui::ClipboardBuffer, const ui::DataTransferEndpoint *, std::u16string *) const override;
    void ReadPng(ui::ClipboardBuffer, const ui::DataTransferEndpoint *, ui::Clipboard::ReadPngCallback) const override;

    std::vector<std::u16string> GetStandardFormats(ui::ClipboardBuffer buffer, const ui::DataTransferEndpoint *data_dst) const override;

    const ui::DataTransferEndpoint *GetSource(ui::ClipboardBuffer buffer) const override;
    void ReadFilenames(ui::ClipboardBuffer buffer,
                       const ui::DataTransferEndpoint *data_dst,
                       std::vector<ui::FileInfo> *result) const override;

protected:
    void WritePortableAndPlatformRepresentations(
        ui::ClipboardBuffer buffer,
        const ObjectMap &objects,
        std::vector<Clipboard::PlatformRepresentation> platform_representations,
        std::unique_ptr<ui::DataTransferEndpoint> data_src) override;
    void WriteText(const char *text_data, size_t text_len) override;
    void WriteHTML(const char *markup_data, size_t markup_len, const char *url_data, size_t url_len) override;
    void WriteRTF(const char *rtf_data, size_t data_len) override;
    void WriteBookmark(const char *title_data, size_t title_len, const char *url_data, size_t url_len) override;
    void WriteWebSmartPaste() override;
    void WriteBitmap(const SkBitmap &bitmap) override;
    void WriteData(const ui::ClipboardFormatType &format, const char *data_data, size_t data_len) override;
    void WriteSvg(const char *, size_t) override;
    void WriteFilenames(std::vector<ui::FileInfo> filenames) override;

    base::flat_map<ui::ClipboardBuffer, std::unique_ptr<ui::DataTransferEndpoint>> m_dataSrc;
};

} // namespace QtWebEngineCore

#endif
