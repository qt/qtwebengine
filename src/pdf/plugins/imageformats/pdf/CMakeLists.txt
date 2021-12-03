qt_internal_add_plugin(QPdfPlugin
    OUTPUT_NAME qpdf
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qpdfiohandler.cpp qpdfiohandler_p.h
    LIBRARIES
        Qt::PdfPrivate
)

