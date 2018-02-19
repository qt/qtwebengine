include(../tests.pri)
include(../../shared/http.pri)
QT *= core-private

qtConfig(webengine-printing-and-pdf): DEFINES+=QWEBENGINEPAGE_PDFPRINTINGENABLED
