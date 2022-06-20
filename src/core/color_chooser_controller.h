// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef COLOR_CHOOSER_CONTROLLER_H
#define COLOR_CHOOSER_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QColor)
QT_FORWARD_DECLARE_CLASS(QVariant)

namespace QtWebEngineCore {

class ColorChooserControllerPrivate;

class Q_WEBENGINECORE_PRIVATE_EXPORT ColorChooserController : public QObject {
    Q_OBJECT
public:
    ~ColorChooserController();

    QColor initialColor() const;

    void didEndColorDialog();
    void didChooseColorInColorDialog(const QColor &);

public Q_SLOTS:
    void accept(const QColor &);
    void accept(const QVariant &);
    void reject();

private:
    ColorChooserController(ColorChooserControllerPrivate *);
    QScopedPointer<ColorChooserControllerPrivate> d;

    friend class ColorChooserQt;
};

} // namespace QtWebEngineCore

#endif // COLOR_CHOOSER_CONTROLLER_H
