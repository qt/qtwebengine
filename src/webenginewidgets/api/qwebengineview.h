// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEVIEW_H
#define QWEBENGINEVIEW_H

#include <QtGui/QPageLayout>
#include <QtGui/qpageranges.h>
#include <QtWidgets/qwidget.h>

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>
#include <QtWebEngineCore/qwebenginepage.h>

namespace QtWebEngineWidgetUI {
class AutofillPopupWidget;
}

QT_BEGIN_NAMESPACE

class QContextMenuEvent;
class QPrinter;
class QUrl;
class QWebEngineContextMenuRequest;
class QWebEngineHistory;
class QWebEngineHttpRequest;
class QWebEngineSettings;
class QWebEngineViewAccessible;
class QWebEngineViewPrivate;

class QWEBENGINEWIDGETS_EXPORT QWebEngineView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QUrl iconUrl READ iconUrl NOTIFY iconUrlChanged)
    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(bool hasSelection READ hasSelection)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)

public:
    explicit QWebEngineView(QWidget *parent = nullptr);
    explicit QWebEngineView(QWebEngineProfile *profile, QWidget *parent = nullptr);
    explicit QWebEngineView(QWebEnginePage *page, QWidget *parent = nullptr);
    virtual ~QWebEngineView();

    static QWebEngineView *forPage(const QWebEnginePage *page);

    QWebEnginePage *page() const;
    void setPage(QWebEnginePage *page);

    void load(const QUrl &url);
    void load(const QWebEngineHttpRequest &request);
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(),
                    const QUrl &baseUrl = QUrl());

    QWebEngineHistory *history() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QUrl iconUrl() const;
    QIcon icon() const;

    bool hasSelection() const;
    QString selectedText() const;

#if QT_CONFIG(action)
    QAction *pageAction(QWebEnginePage::WebAction action) const;
#endif
    void triggerPageAction(QWebEnginePage::WebAction action, bool checked = false);

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);
    void findText(const QString &subString, QWebEnginePage::FindFlags options = {},
                  const std::function<void(const QWebEngineFindTextResult &)> &resultCallback =
                          std::function<void(const QWebEngineFindTextResult &)>());

    QSize sizeHint() const override;
    QWebEngineSettings *settings() const;

#if QT_CONFIG(menu)
    QMenu *createStandardContextMenu();
#endif
    QWebEngineContextMenuRequest *lastContextMenuRequest() const;

    void printToPdf(const QString &filePath,
                    const QPageLayout &layout = QPageLayout(QPageSize(QPageSize::A4),
                                                            QPageLayout::Portrait, QMarginsF()),
                    const QPageRanges &ranges = {});
    void printToPdf(const std::function<void(const QByteArray &)> &resultCallback,
                    const QPageLayout &layout = QPageLayout(QPageSize(QPageSize::A4),
                                                            QPageLayout::Portrait, QMarginsF()),
                    const QPageRanges &ranges = {});
    void print(QPrinter *printer);

public Q_SLOTS:
    void stop();
    void back();
    void forward();
    void reload();

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool);
    void titleChanged(const QString &title);
    void selectionChanged();
    void urlChanged(const QUrl &);
    void iconUrlChanged(const QUrl &);
    void iconChanged(const QIcon &);
    void renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus terminationStatus,
                                 int exitCode);
    void pdfPrintingFinished(const QString &filePath, bool success);
    void printRequested();
    void printFinished(bool success);

protected:
    virtual QWebEngineView *createWindow(QWebEnginePage::WebWindowType type);
#if QT_CONFIG(contextmenu)
    void contextMenuEvent(QContextMenuEvent *) override;
#endif // QT_CONFIG(contextmenu)
    bool event(QEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void closeEvent(QCloseEvent *) override;
#if QT_CONFIG(draganddrop)
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
#endif // QT_CONFIG(draganddrop)

private:
    Q_DISABLE_COPY(QWebEngineView)
    Q_DECLARE_PRIVATE(QWebEngineView)
    QScopedPointer<QWebEngineViewPrivate> d_ptr;

    friend class QtWebEngineWidgetUI::AutofillPopupWidget;
#if QT_CONFIG(accessibility)
    friend class QWebEngineViewAccessible;
#endif
};

QT_END_NAMESPACE

#endif // QWEBENGINEVIEW_H
