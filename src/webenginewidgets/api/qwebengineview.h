/*
    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.

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

#ifndef QWEBENGINEVIEW_H
#define QWEBENGINEVIEW_H

#include <QtGui/qpainter.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtWidgets/qwidget.h>

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>
#include <QtWebEngineWidgets/qwebenginepage.h>

QT_BEGIN_NAMESPACE
class QContextMenuEvent;
class QUrl;
class QWebEnginePage;
class QWebEngineViewPrivate;

class QWEBENGINEWIDGETS_EXPORT QWebEngineView : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(bool hasSelection READ hasSelection)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)

public:
    explicit QWebEngineView(QWidget* parent = 0);
    virtual ~QWebEngineView();

    QWebEnginePage* page() const;
    void setPage(QWebEnginePage* page);

    void load(const QUrl& url);
    void setHtml(const QString& html, const QUrl& baseUrl = QUrl());
    void setContent(const QByteArray& data, const QString& mimeType = QString(), const QUrl& baseUrl = QUrl());

    QWebEngineHistory* history() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;

    bool hasSelection() const;
    QString selectedText() const;

#ifndef QT_NO_ACTION
    QAction* pageAction(QWebEnginePage::WebAction action) const;
#endif
    void triggerPageAction(QWebEnginePage::WebAction action, bool checked = false);

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void findText(const QString &subString, QWebEnginePage::FindFlags options = 0, const QWebEngineCallback<bool> &resultCallback = QWebEngineCallback<bool>());

    virtual QSize sizeHint() const { return QSize(800, 600); }

public Q_SLOTS:
    void stop();
    void back();
    void forward();
    void reload();

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool);
    void titleChanged(const QString& title);
    void selectionChanged();
    void urlChanged(const QUrl&);

protected:
    virtual QWebEngineView *createWindow(QWebEnginePage::WebWindowType type);
    virtual void contextMenuEvent(QContextMenuEvent*) Q_DECL_OVERRIDE;
    virtual bool event(QEvent*) Q_DECL_OVERRIDE;

private:
    Q_DECLARE_PRIVATE(QWebEngineView);

    friend class QWebEnginePage;
    friend class QWebEnginePagePrivate;
};

QT_END_NAMESPACE

#endif // QWEBENGINEVIEW_H
