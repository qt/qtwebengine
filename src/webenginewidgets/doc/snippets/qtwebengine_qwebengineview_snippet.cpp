// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

void wrapInFunction()
{

//! [0]
    view->page()->history();
//! [0]


//! [1]
    view->page()->settings();
//! [1]


//! [2]
    view->triggerPageAction(QWebEnginePage::Copy);
//! [2]


//! [3]
    view->page()->triggerAction(QWebEnginePage::Stop);
//! [3]


//! [4]
    view->page()->triggerAction(QWebEnginePage::Back);
//! [4]


//! [5]
    view->page()->triggerAction(QWebEnginePage::Forward);
//! [5]

//! [6]
    view->page()->settings();
//! [6]
}

