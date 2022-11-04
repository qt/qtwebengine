// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdffile_p.h"

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QPdfFile
    \inmodule QtPdf

    QPdfFile is a means of passing a PDF file along with the associated
    QPdfDocument together into QPdfIOHandler::load(QIODevice *device) so that
    QPdfIOHandler does not need to construct its own redundant QPdfDocument
    instance. If it succeeds in casting the QIODevice to a QPdfFile, it is
    expected to use the QPdfDocument operations for all I/O, and thus the
    normal QFile I/O functions are not needed for that use case.
*/

QPdfFile::QPdfFile(QPdfDocument *doc)
  : QFile(doc->fileName()), m_document(doc)
{
}

QT_END_NAMESPACE

//#include "moc_qpdffile_p.cpp"
