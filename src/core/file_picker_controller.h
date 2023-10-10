// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef FILE_PICKER_CONTROLLER_H
#define FILE_PICKER_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QObject>
#include <QStringList>

namespace QtWebEngineCore {

class FilePickerControllerPrivate;
class Q_WEBENGINECORE_PRIVATE_EXPORT FilePickerController : public QObject {
    Q_OBJECT
public:
    enum FileChooserMode {
        Open,
        OpenMultiple,
        UploadFolder,
        Save
    };

    FilePickerController(FilePickerControllerPrivate *priv, QObject *parent = nullptr);
    ~FilePickerController() override;

    QStringList acceptedMimeTypes() const;
    QString defaultFileName() const;
    FileChooserMode mode() const;

    static QStringList nameFilters(const QStringList &acceptedMimeTypes);

public Q_SLOTS:
    void accepted(const QStringList &files);
    void accepted(const QVariant &files);
    void rejected();

private:
    void filesSelectedInChooser(const QStringList &filesList);
    FilePickerControllerPrivate *d_ptr;
    // Using Quick, the FileSelectListenerImpl destructor may crash in debug mode
    // if the browser window is closed and the FilePicker is still open
    bool m_isHandled = false;
};

} // namespace QtWebEngineCore

#endif // FILE_PICKER_CONTROLLER_H
