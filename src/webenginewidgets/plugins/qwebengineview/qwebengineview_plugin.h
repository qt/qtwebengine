// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEVIEW_PLUGIN_H
#define QWEBENGINEVIEW_PLUGIN_H

#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QtWidgets/QWidget>
#include <QtGui/QIcon>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

namespace fake {
// A lightweight "fake" QWebEngineView class that is returned when Qt Designer
// queries the default property values by calling
// QDesignerCustomWidgetInterface::createWidget() with 0 parent, preventing
// crashes during QWebEngine initialization (QTBUG-53984).
// The property list needs to be kept in sync with QWebEngineView.
class QWebEngineView : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl) // Designable
    Q_PROPERTY(QUrl iconUrl READ iconUrl)
    Q_PROPERTY(QIcon icon READ icon)
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(bool hasSelection READ hasSelection)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor) // Designable

public:
    explicit QWebEngineView(QWidget *parent = nullptr) : QWidget(parent) {}

    QString title() const { return QString(); }
    QUrl url() const { return QUrl(); }
    void setUrl(const QUrl &) {}
    QUrl iconUrl() const { return QUrl(); }
    QIcon icon() const { return QIcon(); }
    QString selectedText() { return QString(); }
    bool hasSelection() { return false; }
    qreal zoomFactor() const { return 1; }
    void setZoomFactor(qreal) {}
};
} // namespace fake

class QWebEngineViewPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    explicit QWebEngineViewPlugin(QObject *parent = nullptr);

    QString name() const override;
    QString group() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QString includeFile() const override;
    QIcon icon() const override;
    bool isContainer() const override;
    QWidget *createWidget(QWidget *parent) override;
    bool isInitialized() const override;
    void initialize(QDesignerFormEditorInterface *core) override;
    QString domXml() const override;

private:
    bool m_initialized;
};

QT_END_NAMESPACE

#endif // QWEBENGINEVIEW_PLUGIN_H
