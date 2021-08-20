qt_internal_add_plugin(QPdfPlugin
    OUTPUT_NAME qpdf
    TYPE imageformats
    SOURCES
        main.cpp
        qpdfiohandler.cpp qpdfiohandler_p.h
    LIBRARIES
        Qt::PdfPrivate
)

