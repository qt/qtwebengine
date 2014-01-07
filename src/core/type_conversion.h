/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TYPE_CONVERSION_H
#define TYPE_CONVERSION_H

#include <QColor>
#include <QMatrix4x4>
#include <QRect>
#include <QString>
#include <QUrl>
#include "base/files/file_path.h"
#include "third_party/skia/include/utils/SkMatrix44.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/rect.h"
#include "ui/shell_dialogs/selected_file_info.h"
#include "url/gurl.h"

inline QString toQt(const base::string16 &string)
{
    return QString::fromUtf16(string.data());
}

inline QString toQt(const std::string &string)
{
    return QString::fromStdString(string);
}

inline base::string16 toString16(const QString &qString)
{
    return base::string16(qString.utf16());
}

inline QUrl toQt(const GURL &url)
{
    return QUrl(QString::fromStdString(url.spec()));
}

inline GURL toGurl(const QUrl& url)
{
    return GURL(url.toString().toStdString());
}

inline QRect toQt(const gfx::Rect &rect)
{
    return QRect(rect.x(), rect.y(), rect.width(), rect.height());
}

inline QRectF toQt(const gfx::RectF &rect)
{
    return QRectF(rect.x(), rect.y(), rect.width(), rect.height());
}

inline QSize toQt(const gfx::Size &size)
{
    return QSize(size.width(), size.height());
}

inline gfx::SizeF toGfx(const QSizeF& size)
{
  return gfx::SizeF(size.width(), size.height());
}

inline QSizeF toQt(const gfx::SizeF &size)
{
    return QSizeF(size.width(), size.height());
}

inline QColor toQt(const SkColor &c)
{
    return QColor(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c), SkColorGetA(c));
}

inline QMatrix4x4 toQt(const SkMatrix44 &m)
{
    return QMatrix4x4(
        m.get(0, 0), m.get(0, 1), m.get(0, 2), m.get(0, 3),
        m.get(1, 0), m.get(1, 1), m.get(1, 2), m.get(1, 3),
        m.get(2, 0), m.get(2, 1), m.get(2, 2), m.get(2, 3),
        m.get(3, 0), m.get(3, 1), m.get(3, 2), m.get(3, 3));
}

inline base::FilePath::StringType toFilePathString(const QString &str)
{
#if defined(OS_POSIX)
    return str.toStdString();
#elif defined(OS_WIN)
    return str.toStdWString();
#endif
}

template <typename T>
inline T fileListingHelper(const QString &) {qFatal("Specialization missing for %s.", Q_FUNC_INFO);}

template <>
inline ui::SelectedFileInfo fileListingHelper<ui::SelectedFileInfo>(const QString &file)
{
    base::FilePath fp(toFilePathString(file));
    return ui::SelectedFileInfo(fp, fp);
}

template <>
inline base::FilePath fileListingHelper<base::FilePath>(const QString &file)
{
    return base::FilePath(toFilePathString(file));
}


template <typename T>
inline std::vector<T> toVector(const QStringList &fileList)
{
    std::vector<T> selectedFiles;
    selectedFiles.reserve(fileList.size());
    Q_FOREACH (const QString &file, fileList)
        selectedFiles.push_back(fileListingHelper<T>(file));
    return selectedFiles;
}

#endif // TYPE_CONVERSION_H
