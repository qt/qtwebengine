TEMPLATE = subdirs

SUBDIRS = \
    qpdfbookmarkmodel \
    qpdfpagenavigation \
    qpdfpagerenderer

qtHaveModule(printsupport): SUBDIRS += qpdfdocument
