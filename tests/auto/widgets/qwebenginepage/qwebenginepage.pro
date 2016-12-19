include(../tests.pri)
QT *= core-private gui-private

contains(WEBENGINE_CONFIG, use_pdf): DEFINES+=QWEBENGINEPAGE_PDFPRINTINGENABLED
