/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef FILE_PICKER_CONTROLLER_H
#define FILE_PICKER_CONTROLLER_H

#include "qtwebenginecoreglobal.h"
#include <QObject>
#include <QStringList>

namespace content {
    class WebContents;
}

namespace QtWebEngineCore {

class QWEBENGINE_EXPORT FilePickerController : public QObject {
    Q_OBJECT
public:
    enum FileChooserMode {
        Open,
        OpenMultiple,
        UploadFolder,
        Save
    };

    FilePickerController(FileChooserMode mode, content::WebContents *contents, const QString &defaultFileName, const QStringList &acceptedMimeTypes, QObject * = 0);
    QStringList acceptedMimeTypes();
    QString defaultFileName();
    FileChooserMode mode();
    void filesSelectedInChooser(const QStringList &filesList, content::WebContents *contents);

public Q_SLOTS:
    void accepted(const QStringList &files);
    void accepted(const QVariant &files);
    void rejected();

private:
    QString m_defaultFileName;
    QStringList m_acceptedMimeTypes;
    content::WebContents *m_contents;
    FileChooserMode m_mode;

};

} // namespace

#endif // FILE_PICKER_CONTROLLER_H
