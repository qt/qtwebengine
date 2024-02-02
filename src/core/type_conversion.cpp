// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "type_conversion.h"

#include "base/containers/contains.h"
#include <components/favicon_base/favicon_util.h>
#include <net/cert/x509_certificate.h>
#include <net/cert/x509_util.h>
#include <ui/events/event_constants.h>
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "third_party/blink/public/mojom/favicon/favicon_url.mojom.h"

#include <QtCore/qcoreapplication.h>
#include <QtNetwork/qsslcertificate.h>

namespace QtWebEngineCore {

QImage toQImage(const SkBitmap &bitmap)
{
    QImage image;
    switch (bitmap.colorType()) {
    case kUnknown_SkColorType:
    case kRGBA_F16_SkColorType:
    case kRGBA_F32_SkColorType:
    case kRGBA_F16Norm_SkColorType:
    case kR8G8_unorm_SkColorType:
    case kA16_float_SkColorType:
    case kA16_unorm_SkColorType:
    case kR16G16_float_SkColorType:
    case kR16G16_unorm_SkColorType:
    case kR8_unorm_SkColorType:
        qWarning("Unknown or unsupported skia image format");
        break;
    case kAlpha_8_SkColorType:
        image = toQImage(bitmap, QImage::Format_Alpha8);
        break;
    case kRGB_565_SkColorType:
        image = toQImage(bitmap, QImage::Format_RGB16);
        break;
    case kARGB_4444_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kUnpremul_SkAlphaType:
            // not supported - treat as opaque
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGB444);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_ARGB4444_Premultiplied);
            break;
        }
        break;
    case kSRGBA_8888_SkColorType:
    case kRGB_888x_SkColorType:
    case kRGBA_8888_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBX8888);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBA8888_Premultiplied);
            break;
        case kUnpremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBA8888);
            break;
        }
        break;
    case kBGRA_8888_SkColorType:
        // we are assuming little-endian arch here.
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGB32);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_ARGB32_Premultiplied);
            break;
        case kUnpremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_ARGB32);
            break;
        }
        break;
    case kRGB_101010x_SkColorType:
    case kRGBA_1010102_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kUnpremul_SkAlphaType:
            // not supported - treat as opaque
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGB30);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_A2RGB30_Premultiplied);
            break;
        }
        break;
    case kBGR_101010x_SkColorType:
    case kBGR_101010x_XR_SkColorType:
    case kBGRA_1010102_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kUnpremul_SkAlphaType:
            // not supported - treat as opaque
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_BGR30);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_A2BGR30_Premultiplied);
            break;
        }
        break;
    case kGray_8_SkColorType:
        image = toQImage(bitmap, QImage::Format_Grayscale8);
        break;
    case kR16G16B16A16_unorm_SkColorType:
        switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType:
            break;
        case kUnpremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBA64);
            break;
        case kOpaque_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBX64);
            break;
        case kPremul_SkAlphaType:
            image = toQImage(bitmap, QImage::Format_RGBA64_Premultiplied);
            break;
        }
        break;
    }
    return image;
}

QImage toQImage(const gfx::ImageSkiaRep &imageSkiaRep)
{
    QImage image = toQImage(imageSkiaRep.GetBitmap());
    if (!image.isNull() && imageSkiaRep.scale() != 1.0f)
        image.setDevicePixelRatio(imageSkiaRep.scale());
    return image;
}

SkBitmap toSkBitmap(const QImage &image)
{
    SkBitmap bitmap;
    SkImageInfo imageInfo;

    switch (image.format()) {
    case QImage::Format_RGB32:
        imageInfo = SkImageInfo::Make(image.width(), image.height(), kBGRA_8888_SkColorType, kOpaque_SkAlphaType);
        break;
    case QImage::Format_ARGB32:
        imageInfo = SkImageInfo::Make(image.width(), image.height(), kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
        break;
    case QImage::Format_ARGB32_Premultiplied:
        imageInfo = SkImageInfo::Make(image.width(), image.height(), kBGRA_8888_SkColorType, kPremul_SkAlphaType);
        break;
    case QImage::Format_RGBX8888:
        imageInfo = SkImageInfo::Make(image.width(), image.height(), kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        break;
    case QImage::Format_RGBA8888:
        imageInfo = SkImageInfo::Make(image.width(), image.height(), kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
        break;
    case QImage::Format_RGBA8888_Premultiplied:
        imageInfo = SkImageInfo::Make(image.width(), image.height(), kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        break;
    default:
        return toSkBitmap(image.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    }

    bitmap.installPixels(imageInfo, (void *)image.bits(), image.bytesPerLine());

    // Ensure we copy the pixels
    SkBitmap bitmapCopy;
    bitmapCopy.allocPixels(imageInfo);
    bitmapCopy.writePixels(bitmap.pixmap());

    return bitmapCopy;
}

QIcon toQIcon(const gfx::Image &image)
{
    // Based on ExtractSkBitmapsToStore in chromium/components/favicon/core/favicon_service_impl.cc
    gfx::ImageSkia image_skia = image.AsImageSkia();
    image_skia.EnsureRepsForSupportedScales();
    const std::vector<gfx::ImageSkiaRep> &image_reps = image_skia.image_reps();
    std::vector<SkBitmap> bitmaps;
    const std::vector<float> favicon_scales = favicon_base::GetFaviconScales();
    for (size_t i = 0; i < image_reps.size(); ++i) {
        // Don't save if the scale isn't one of supported favicon scales.
        if (!base::Contains(favicon_scales, image_reps[i].scale()))
            continue;
        bitmaps.push_back(image_reps[i].GetBitmap());
    }
    return toQIcon(bitmaps);
}

QIcon toQIcon(const std::vector<SkBitmap> &bitmaps)
{
    if (!bitmaps.size())
        return QIcon();

    QIcon icon;

    for (unsigned i = 0; i < bitmaps.size(); ++i) {
        SkBitmap bitmap = bitmaps[i];
        QImage image = toQImage(bitmap);

        icon.addPixmap(QPixmap::fromImage(image).copy());
    }

    return icon;
}

int flagsFromModifiers(Qt::KeyboardModifiers modifiers)
{
    int modifierFlags = ui::EF_NONE;
#if defined(Q_OS_OSX)
    if (!qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
        if ((modifiers & Qt::ControlModifier) != 0)
            modifierFlags |= ui::EF_COMMAND_DOWN;
        if ((modifiers & Qt::MetaModifier) != 0)
            modifierFlags |= ui::EF_CONTROL_DOWN;
    } else
#endif
    {
        if ((modifiers & Qt::ControlModifier) != 0)
            modifierFlags |= ui::EF_CONTROL_DOWN;
        if ((modifiers & Qt::MetaModifier) != 0)
            modifierFlags |= ui::EF_COMMAND_DOWN;
    }
    if ((modifiers & Qt::ShiftModifier) != 0)
        modifierFlags |= ui::EF_SHIFT_DOWN;
    if ((modifiers & Qt::AltModifier) != 0)
        modifierFlags |= ui::EF_ALT_DOWN;
    return modifierFlags;
}

static QSslCertificate toCertificate(CRYPTO_BUFFER *buffer)
{
    auto derCert = net::x509_util::CryptoBufferAsStringPiece(buffer);
    return QSslCertificate(QByteArray::fromRawData(derCert.data(), derCert.size()), QSsl::Der);
}

QList<QSslCertificate> toCertificateChain(net::X509Certificate *certificate)
{
    // from leaf to root as in QtNetwork
    QList<QSslCertificate> chain;
    chain.append(toCertificate(certificate->cert_buffer()));
    for (auto &&buffer : certificate->intermediate_buffers())
        chain.append(toCertificate(buffer.get()));
    return chain;
}

Qt::InputMethodHints toQtInputMethodHints(ui::TextInputType inputType)
{
    switch (inputType) {
    case ui::TEXT_INPUT_TYPE_TEXT:
        return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_SEARCH:
        return Qt::ImhPreferLowercase | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
        return Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase
                | Qt::ImhHiddenText;
    case ui::TEXT_INPUT_TYPE_EMAIL:
        return Qt::ImhEmailCharactersOnly;
    case ui::TEXT_INPUT_TYPE_NUMBER:
        return Qt::ImhFormattedNumbersOnly;
    case ui::TEXT_INPUT_TYPE_TELEPHONE:
        return Qt::ImhDialableCharactersOnly;
    case ui::TEXT_INPUT_TYPE_URL:
        return Qt::ImhUrlCharactersOnly | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_DATE_TIME:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_FIELD:
        return Qt::ImhDate | Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_DATE:
    case ui::TEXT_INPUT_TYPE_MONTH:
    case ui::TEXT_INPUT_TYPE_WEEK:
        return Qt::ImhDate;
    case ui::TEXT_INPUT_TYPE_TIME:
        return Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_TEXT_AREA:
    case ui::TEXT_INPUT_TYPE_CONTENT_EDITABLE:
        return Qt::ImhMultiLine | Qt::ImhPreferLowercase;
    default:
        return Qt::ImhNone;
    }
}

} // namespace QtWebEngineCore
