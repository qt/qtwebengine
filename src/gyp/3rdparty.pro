TEMPLATE = aux

pdfium_pri.target = pdfium.pri
pdfium_pri.commands = python gyp2pri.py pdfium/pdfium.gyp pdfium pdfium.pri
pdfium_pri.depends = pdfium/pdfium.gyp

QMAKE_EXTRA_TARGETS = pdfium_pri
PRE_TARGETDEPS = $$pdfium_pri.target
