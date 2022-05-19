// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TYPE_CONVERSION_H
#define TYPE_CONVERSION_H

#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QIcon>
#include <QImage>
#include <QNetworkCookie>
#include <QRect>
#include <QString>
#include <QUrl>
#include "base/files/file_path.h"
#include "base/time/time.h"
#include "net/cookies/canonical_cookie.h"
#include "third_party/blink/public/mojom/favicon/favicon_url.mojom-forward.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "url/gurl.h"
#include "url/origin.h"

QT_FORWARD_DECLARE_CLASS(QSslCertificate)

namespace gfx {
class Image;
class ImageSkiaRep;
}

namespace net {
class X509Certificate;
}

namespace QtWebEngineCore {

#if defined(Q_OS_WIN)
inline QString toQt(const std::wstring &string)
{
    return QString::fromStdWString(string);
}
#endif

inline QString toQt(const std::u16string &string)
{
    return QString::fromStdU16String(string);
}

inline QString toQt(const absl::optional<std::u16string> &string)
{
    if (!string.has_value())
        return QString();
    return QString::fromStdU16String(*string);
}

inline QString toQString(const std::string &string)
{
    return QString::fromStdString(string);
}

inline QByteArray toQByteArray(const std::string &string)
{
    return QByteArray::fromStdString(string);
}

// ### should probably be toQByteArray
inline QString toQt(const std::string &string)
{
    return toQString(string);
}

inline std::u16string toString16(const QString &qString)
{
    return qString.toStdU16String();
}

inline absl::optional<std::u16string> toOptionalString16(const QString &qString)
{
    if (qString.isNull())
        return absl::nullopt;
    return absl::make_optional(qString.toStdU16String());
}

inline QUrl toQt(const GURL &url)
{
    if (url.is_valid())
        return QUrl::fromEncoded(toQByteArray(url.spec()));

    return QUrl(toQString(url.possibly_invalid_spec()));
}

inline GURL toGurl(const QUrl& url)
{
    return GURL(url.toEncoded().toStdString());
}

inline QUrl toQt(const url::Origin &origin)
{
    return QUrl::fromEncoded(toQByteArray(origin.Serialize()));
}

inline url::Origin toOrigin(const QUrl &url)
{
    return url::Origin::Create(toGurl(url));
}

inline QPoint toQt(const gfx::Point &point)
{
    return QPoint(point.x(), point.y());
}

inline QPointF toQt(const gfx::PointF &point)
{
    return QPointF(point.x(), point.y());
}

inline QPointF toQt(const gfx::Vector2dF &point)
{
    return QPointF(point.x(), point.y());
}

inline gfx::Point toGfx(const QPoint& point)
{
  return gfx::Point(point.x(), point.y());
}

inline gfx::PointF toGfx(const QPointF& point)
{
  return gfx::PointF(point.x(), point.y());
}

inline QRect toQt(const gfx::Rect &rect)
{
    return QRect(rect.x(), rect.y(), rect.width(), rect.height());
}

inline QRectF toQt(const gfx::RectF &rect)
{
    return QRectF(rect.x(), rect.y(), rect.width(), rect.height());
}

inline gfx::Size toGfx(const QSize &size)
{
    return gfx::Size(size.width(), size.height());
}

inline QSize toQt(const gfx::Size &size)
{
    return QSize(size.width(), size.height());
}

inline gfx::SizeF toGfx(const QSizeF& size)
{
  return gfx::SizeF(size.width(), size.height());
}

inline gfx::Rect toGfx(const QRect &rect)
{
    return gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

inline QSizeF toQt(const gfx::SizeF &size)
{
    return QSizeF(size.width(), size.height());
}

inline QSize toQt(const SkISize &size)
{
    return QSize(size.width(), size.height());
}

inline QColor toQt(const SkColor &c)
{
    return QColor(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c), SkColorGetA(c));
}

inline SkColor toSk(const QColor &c)
{
    return c.rgba();
}

inline QImage toQImage(const SkBitmap &bitmap, QImage::Format format)
{
    SkPixelRef *pixelRef = bitmap.pixelRef();
    return QImage((uchar *)pixelRef->pixels(), bitmap.width(), bitmap.height(), format);
}

QImage toQImage(const SkBitmap &bitmap);
QImage toQImage(const gfx::ImageSkiaRep &imageSkiaRep);
SkBitmap toSkBitmap(const QImage &image);

QIcon toQIcon(const gfx::Image &image);
QIcon toQIcon(const std::vector<SkBitmap> &bitmaps);

inline QDateTime toQt(base::Time time)
{
    return QDateTime::fromMSecsSinceEpoch(time.ToJavaTime());
}

inline base::Time toTime(const QDateTime &dateTime) {
    return base::Time::FromJavaTime(dateTime.toMSecsSinceEpoch());
}

inline QNetworkCookie toQt(const net::CanonicalCookie & cookie)
{
    QNetworkCookie qCookie = QNetworkCookie(QByteArray::fromStdString(cookie.Name()), QByteArray::fromStdString(cookie.Value()));
    qCookie.setDomain(toQt(cookie.Domain()));
    if (!cookie.ExpiryDate().is_null())
        qCookie.setExpirationDate(toQt(cookie.ExpiryDate()));
    qCookie.setHttpOnly(cookie.IsHttpOnly());
    qCookie.setPath(toQt(cookie.Path()));
    qCookie.setSecure(cookie.IsSecure());
    return qCookie;
}

inline base::FilePath::StringType toFilePathString(const QString &str)
{
#if defined(Q_OS_WIN)
    return QDir::toNativeSeparators(str).toStdWString();
#else
    return str.toStdString();
#endif
}

inline base::FilePath toFilePath(const QString &str)
{
    return base::FilePath(toFilePathString(str));
}

int flagsFromModifiers(Qt::KeyboardModifiers modifiers);

inline QStringList fromVector(const std::vector<std::u16string> &vector)
{
    QStringList result;
    for (auto s: vector) {
      result.append(toQt(s));
    }
    return result;
}

QList<QSslCertificate> toCertificateChain(net::X509Certificate *certificate);

Qt::InputMethodHints toQtInputMethodHints(ui::TextInputType inputType);

} // namespace QtWebEngineCore

#endif // TYPE_CONVERSION_H
