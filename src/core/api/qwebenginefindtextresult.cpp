/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "qwebenginefindtextresult.h"

QT_BEGIN_NAMESPACE

class QWebEngineFindTextResultPrivate : public QSharedData {
public:
    int numberOfMatches = 0;
    int activeMatch = 0;
};

/*!
    \class QWebEngineFindTextResult
    \brief The QWebEngineFindTextResult class encapsulates the result of a string search on a page.
    \since 5.14

    \inmodule QtWebEngineCore

    The string search can be initiated by the \l QWebEnginePage::findText() or
    \l{WebEngineView::findText()}{WebEngineView.findText()} method. The results of the search
    are highlighted in the view. The details of this result are passed as a
    QWebEngineFindTextResult object that can be used to show a status message,
    such as "2 of 2 matches". For example:

    \code
    QObject::connect(view.page(), &QWebEnginePage::findTextFinished, [](const QWebEngineFindTextResult &result) {
        qInfo() << result.activeMatch() << "of" << result.numberOfMatches() << "matches";
    });
    \endcode

    Results are passed to the user in the
    \l QWebEnginePage::findTextFinished() and
    \l{WebEngineView::findTextFinished()}{WebEngineView.findTextFinished()} signals.
*/

/*! \internal
*/
QWebEngineFindTextResult::QWebEngineFindTextResult()
    : d(new QWebEngineFindTextResultPrivate)
{}

/*! \internal
*/
QWebEngineFindTextResult::QWebEngineFindTextResult(int numberOfMatches, int activeMatch)
    : d(new QWebEngineFindTextResultPrivate)
{
    d->numberOfMatches = numberOfMatches;
    d->activeMatch = activeMatch;
}

/*! \internal
*/
QWebEngineFindTextResult::QWebEngineFindTextResult(const QWebEngineFindTextResult &other)
    : d(other.d)
{}

/*! \internal
*/
QWebEngineFindTextResult &QWebEngineFindTextResult::operator=(const QWebEngineFindTextResult &other)
{
    d = other.d;
    return *this;
}

/*! \internal
*/
QWebEngineFindTextResult::~QWebEngineFindTextResult()
{}

/*!
    \property QWebEngineFindTextResult::numberOfMatches
    \brief The number of matches found.
*/
int QWebEngineFindTextResult::numberOfMatches() const
{
    return d->numberOfMatches;
}

/*!
    \property QWebEngineFindTextResult::activeMatch
    \brief The index of the currently highlighted match.
*/
int QWebEngineFindTextResult::activeMatch() const
{
    return d->activeMatch;
}

QT_END_NAMESPACE

#include "moc_qwebenginefindtextresult.cpp"
