# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Qml Quick)

set(qml_files
    "+Material/PdfStyle.qml"
    "+Universal/PdfStyle.qml"
    "PdfLinkDelegate.qml"
    "PdfMultiPageView.qml"
    "PdfPageView.qml"
    "PdfScrollablePageView.qml"
    "PdfStyle.qml"
)

qt_internal_add_qml_module(PdfQuick
    URI "QtQuick.Pdf"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 5
    QML_FILES ${qml_files}
    DEPENDENCIES QtQuick/auto
    SOURCES
        qquickpdfbookmarkmodel.cpp qquickpdfbookmarkmodel_p.h
        qquickpdfdocument.cpp qquickpdfdocument_p.h
        qquickpdflinkmodel.cpp qquickpdflinkmodel_p.h
        qquickpdfpagenavigator.cpp qquickpdfpagenavigator_p.h
        qquickpdfpageimage.cpp qquickpdfpageimage_p.h
        qquickpdfsearchmodel.cpp qquickpdfsearchmodel_p.h
        qquickpdfselection.cpp qquickpdfselection_p.h
        qtpdfquickglobal_p.h
    INCLUDE_DIRECTORIES
        ../3rdparty/chromium
    PUBLIC_LIBRARIES
        Qt::QuickPrivate
        Qt::PdfPrivate
        Qt::Core
        Qt::Gui
        Qt::Qml
)

