include(../tests.pri)
QT *= core-private

qtConfig(printing-and-pdf): DEFINES+=QWEBENGINEPAGE_PDFPRINTINGENABLED
