TEMPLATE = subdirs

SUBDIRS = qpdfbookmarkmodel
qtHaveModule(printsupport): SUBDIRS += qpdfdocument
