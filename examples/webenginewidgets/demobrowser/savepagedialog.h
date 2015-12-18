/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SAVEPAGEDIALOG_H
#define SAVEPAGEDIALOG_H

#include <QtWidgets/QDialog>
#include <QtWebEngineWidgets/QWebEngineDownloadItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class SavePageDialog;
}
QT_END_NAMESPACE

class SavePageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SavePageDialog(QWidget *parent, QWebEngineDownloadItem::SavePageFormat format,
                            const QString &filePath);
    ~SavePageDialog();

    QWebEngineDownloadItem::SavePageFormat pageFormat() const;
    QString filePath() const;

private slots:
    void on_chooseFilePathButton_clicked();
    void on_formatComboBox_currentIndexChanged(int idx);

private:
    static int formatToIndex(QWebEngineDownloadItem::SavePageFormat format);
    static QWebEngineDownloadItem::SavePageFormat indexToFormat(int idx);
    static QString suffixOfFormat(QWebEngineDownloadItem::SavePageFormat format);
    void setFilePath(const QString &filePath);
    void ensureFileSuffix(QWebEngineDownloadItem::SavePageFormat format);

    static const QWebEngineDownloadItem::SavePageFormat m_indexToFormatTable[];
    Ui::SavePageDialog *ui;
};

#endif // SAVEPAGEDIALOG_H
