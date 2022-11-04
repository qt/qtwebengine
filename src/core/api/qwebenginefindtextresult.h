// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEFINDTEXTRESULT_H
#define QWEBENGINEFINDTEXTRESULT_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>

namespace QtWebEngineCore {
class FindTextHelper;
}

QT_BEGIN_NAMESPACE

class QWebEngineFindTextResultPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineFindTextResult
{
    Q_GADGET
    Q_PROPERTY(int numberOfMatches READ numberOfMatches CONSTANT FINAL)
    Q_PROPERTY(int activeMatch READ activeMatch CONSTANT FINAL)

public:
    int numberOfMatches() const;
    int activeMatch() const;

    QWebEngineFindTextResult();
    QWebEngineFindTextResult(const QWebEngineFindTextResult &other);
    QWebEngineFindTextResult &operator=(const QWebEngineFindTextResult &other);
    ~QWebEngineFindTextResult();

private:
    QWebEngineFindTextResult(int numberOfMatches, int activeMatch);

    QSharedDataPointer<QWebEngineFindTextResultPrivate> d;

    friend class QtWebEngineCore::FindTextHelper;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QWebEngineFindTextResult)

#endif // QWEBENGINEFINDTEXTRESULT_H
