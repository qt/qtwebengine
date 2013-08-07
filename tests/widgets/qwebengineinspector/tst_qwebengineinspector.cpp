/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtTest/QtTest>

#include <qdir.h>
#include <qwebengineinspector.h>
#include <qwebenginepage.h>
#include <qwebenginesettings.h>

class tst_QWebEngineInspector : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void attachAndDestroyPageFirst();
    void attachAndDestroyInspectorFirst();
    void attachAndDestroyInternalInspector();
};

void tst_QWebEngineInspector::attachAndDestroyPageFirst()
{
    // External inspector + manual destruction of page first
    QWebEnginePage* page = new QWebEnginePage();
    page->settings()->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, true);
    QWebEngineInspector* inspector = new QWebEngineInspector();
    inspector->setPage(page);
    page->updatePositionDependentActions(QPoint(0, 0));
    page->triggerAction(QWebEnginePage::InspectElement);

    delete page;
    delete inspector;
}

void tst_QWebEngineInspector::attachAndDestroyInspectorFirst()
{
    // External inspector + manual destruction of inspector first
    QWebEnginePage* page = new QWebEnginePage();
    page->settings()->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, true);
    QWebEngineInspector* inspector = new QWebEngineInspector();
    inspector->setPage(page);
    page->updatePositionDependentActions(QPoint(0, 0));
    page->triggerAction(QWebEnginePage::InspectElement);

    delete inspector;
    delete page;
}

void tst_QWebEngineInspector::attachAndDestroyInternalInspector()
{
    // Internal inspector
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, true);
    page.updatePositionDependentActions(QPoint(0, 0));
    page.triggerAction(QWebEnginePage::InspectElement);
}

QTEST_MAIN(tst_QWebEngineInspector)

#include "tst_qwebengineinspector.moc"
