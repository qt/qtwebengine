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

#ifndef QQUICKWEBENGINECONTEXTMENUDATA_P_H
#define QQUICKWEBENGINECONTEXTMENUDATA_P_H

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

#include <private/qtwebengineglobal_p.h>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtQuick/QQuickItem>

namespace QtWebEngineCore {
class WebEngineContextMenuData;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineView;
class QQuickWebEngineViewPrivate;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineContextMenuData : public QObject {
    Q_OBJECT
public:
    QQuickWebEngineContextMenuData();
    ~QQuickWebEngineContextMenuData();

    enum MediaType {
        MediaTypeNone,
        MediaTypeImage,
        MediaTypeVideo,
        MediaTypeAudio,
        MediaTypeCanvas,
        MediaTypeFile,
        MediaTypePlugin
    };
    Q_ENUM(MediaType)

    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectedTextChanged)
    Q_PROPERTY(QString linkText READ linkText NOTIFY linkTextChanged)
    Q_PROPERTY(QUrl linkUrl READ linkUrl NOTIFY linkUrlChanged)
    Q_PROPERTY(QUrl mediaUrl READ mediaUrl NOTIFY mediaUrlChanged)
    Q_PROPERTY(MediaType mediaType READ mediaType NOTIFY mediaTypeChanged)
    Q_PROPERTY(bool isContentEditable READ isContentEditable NOTIFY isContentEditableChanged)

    bool isValid() const;

    QPoint position() const;
    QString selectedText() const;
    QString linkText() const;
    QUrl linkUrl() const;
    QUrl mediaUrl() const;
    MediaType mediaType() const;
    bool isContentEditable() const;

Q_SIGNALS:
    void isValidChanged();
    void positionChanged();
    void selectedTextChanged();
    void linkTextChanged();
    void linkUrlChanged();
    void mediaUrlChanged();
    void mediaTypeChanged();
    void isContentEditableChanged();

private:
    void update(const QtWebEngineCore::WebEngineContextMenuData &update);

    friend class QQuickWebEngineView;
    friend class QQuickWebEngineViewPrivate;
    Q_DISABLE_COPY(QQuickWebEngineContextMenuData)
    typedef QtWebEngineCore::WebEngineContextMenuData QQuickWebEngineContextMenuDataPrivate;
    QQuickWebEngineContextMenuData(const QQuickWebEngineContextMenuDataPrivate *priv, QObject *parent = 0);
    QQuickWebEngineContextMenuData &operator=(const QQuickWebEngineContextMenuDataPrivate *priv);
    const QQuickWebEngineContextMenuDataPrivate *d;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(const QQuickWebEngineContextMenuData);

#endif // QQUICKWEBENGINECONTEXTMENUDATA_P_H
