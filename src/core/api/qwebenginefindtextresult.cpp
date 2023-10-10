// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
