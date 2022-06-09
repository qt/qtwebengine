// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFIOHANDLER_H
#define QPDFIOHANDLER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qimageiohandler.h>
#include <QtPdf/QPdfDocument>

QT_BEGIN_NAMESPACE

class QPdfIOHandler : public QImageIOHandler
{
public:
    QPdfIOHandler();
    ~QPdfIOHandler() override;
    bool canRead() const override;
    static bool canRead(QIODevice *device);
    int currentImageNumber() const override;
    QRect currentImageRect() const override;
    int imageCount() const override;
    bool read(QImage *image) override;
    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant & value) override;
    bool supportsOption(ImageOption option) const override;
    bool jumpToImage(int frame) override;
    bool jumpToNextImage() override;

private:
    bool load(QIODevice *device);

private:
    QPdfDocument *m_doc = nullptr;
    int m_page = -1;

    QRect m_clipRect;
    QSize m_scaledSize;
    QRect m_scaledClipRect;
    QColor m_backColor = Qt::transparent;
    bool m_loaded = false;
    bool m_ownsDocument = false;
};

QT_END_NAMESPACE

#endif // QPDFIOHANDLER_H
