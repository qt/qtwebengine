/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QWEBENGINECALLBACK_H
#define QWEBENGINECALLBACK_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

namespace QtWebEngineCore {
class CallbackDirectory;
}

QT_BEGIN_NAMESPACE

namespace QtWebEnginePrivate {

template <typename T>
class QWebEngineCallbackPrivateBase : public QSharedData {
public:
    QWebEngineCallbackPrivateBase() {}
    virtual ~QWebEngineCallbackPrivateBase() {}
    virtual void operator()(T) = 0;
};

template <typename T, typename F>
class QWebEngineCallbackPrivate : public QWebEngineCallbackPrivateBase<T> {
public:
    QWebEngineCallbackPrivate(F callable)
        : m_callable(callable)
    {}
    virtual void operator()(T value) Q_DECL_OVERRIDE { m_callable(value); }
private:
    F m_callable;
};

} // namespace QtWebEnginePrivate

template <typename T>
class QWebEngineCallback {
public:
    template <typename F>
    QWebEngineCallback(F f)
        : d(new QtWebEnginePrivate::QWebEngineCallbackPrivate<T, F>(f))
    { }
    QWebEngineCallback() { }
    void swap(QWebEngineCallback &other) Q_DECL_NOTHROW { qSwap(d, other.d); }
    operator bool() const { return d; }
private:
    friend class QtWebEngineCore::CallbackDirectory;
    QExplicitlySharedDataPointer<QtWebEnginePrivate::QWebEngineCallbackPrivateBase<T> > d;
};

Q_DECLARE_SHARED(QWebEngineCallback<int>)
Q_DECLARE_SHARED(QWebEngineCallback<const QByteArray &>)
#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
Q_DECLARE_SHARED_NOT_MOVABLE_UNTIL_QT6(QWebEngineCallback<bool>)
Q_DECLARE_SHARED_NOT_MOVABLE_UNTIL_QT6(QWebEngineCallback<const QString &>)
Q_DECLARE_SHARED_NOT_MOVABLE_UNTIL_QT6(QWebEngineCallback<const QVariant &>)
#endif

QT_END_NAMESPACE

#endif // QWEBENGINECALLBACK_H
