/*
    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

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

#ifndef QWEBENGINEINSPECTOR_H
#define QWEBENGINEINSPECTOR_H

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>

QT_BEGIN_NAMESPACE

class QWebEnginePage;
class QWebEngineInspectorPrivate;

class QWEBENGINEWIDGETS_EXPORT QWebEngineInspector : public QWidget {
    Q_OBJECT
public:
    QWebEngineInspector(QWidget* parent = 0);
    ~QWebEngineInspector();

    void setPage(QWebEnginePage* page);
    QWebEnginePage* page() const;
};

QT_END_NAMESPACE

#endif // QWEBENGINEINSPECTOR_H
