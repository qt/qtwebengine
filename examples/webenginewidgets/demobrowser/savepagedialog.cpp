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

#include "savepagedialog.h"
#include "ui_savepagedialog.h"

#include <QtCore/QDir>
#include <QtWidgets/QFileDialog>

const QWebEngineDownloadItem::SavePageFormat SavePageDialog::m_indexToFormatTable[] = {
    QWebEngineDownloadItem::SingleHtmlSaveFormat,
    QWebEngineDownloadItem::CompleteHtmlSaveFormat,
    QWebEngineDownloadItem::MimeHtmlSaveFormat
};

SavePageDialog::SavePageDialog(QWidget *parent, QWebEngineDownloadItem::SavePageFormat format,
                               const QString &filePath)
    : QDialog(parent)
    , ui(new Ui::SavePageDialog)
{
    ui->setupUi(this);
    ui->formatComboBox->setCurrentIndex(formatToIndex(format));
    setFilePath(filePath);
}

SavePageDialog::~SavePageDialog()
{
    delete ui;
}

QWebEngineDownloadItem::SavePageFormat SavePageDialog::pageFormat() const
{
    return indexToFormat(ui->formatComboBox->currentIndex());
}

QString SavePageDialog::filePath() const
{
    return QDir::fromNativeSeparators(ui->filePathLineEdit->text());
}

void SavePageDialog::on_chooseFilePathButton_clicked()
{
    QFileInfo fi(filePath());
    QFileDialog dlg(this, tr("Save Page As"), fi.absolutePath());
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setDefaultSuffix(suffixOfFormat(pageFormat()));
    dlg.selectFile(fi.absoluteFilePath());
    if (dlg.exec() != QDialog::Accepted)
        return;
    setFilePath(dlg.selectedFiles().first());
    ensureFileSuffix(pageFormat());
}

void SavePageDialog::on_formatComboBox_currentIndexChanged(int idx)
{
    ensureFileSuffix(indexToFormat(idx));
}

int SavePageDialog::formatToIndex(QWebEngineDownloadItem::SavePageFormat format)
{
    for (auto i : m_indexToFormatTable) {
        if (m_indexToFormatTable[i] == format)
            return i;
    }
    Q_UNREACHABLE();
}

QWebEngineDownloadItem::SavePageFormat SavePageDialog::indexToFormat(int idx)
{
    Q_ASSERT(idx >= 0 && size_t(idx) < (sizeof(m_indexToFormatTable)
                                        / sizeof(QWebEngineDownloadItem::SavePageFormat)));
    return m_indexToFormatTable[idx];
}

QString SavePageDialog::suffixOfFormat(QWebEngineDownloadItem::SavePageFormat format)
{
    if (format == QWebEngineDownloadItem::MimeHtmlSaveFormat)
        return QStringLiteral(".mhtml");
    return QStringLiteral(".html");
}

void SavePageDialog::setFilePath(const QString &filePath)
{
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
}

void SavePageDialog::ensureFileSuffix(QWebEngineDownloadItem::SavePageFormat format)
{
    QFileInfo fi(filePath());
    setFilePath(fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName()
                + suffixOfFormat(format));
}
