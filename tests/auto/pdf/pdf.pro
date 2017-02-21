TEMPLATE = subdirs

SUBDIRS = \
    qpdfbookmarkmodel \
    qpdfpagenavigation

qtHaveModule(printsupport): SUBDIRS += qpdfdocument
