TEMPLATE = subdirs
pdfcore.file = pdfcore.pro
pdfcore_generator.file = pdfcore_generator.pro
gn_run.file = gn_run.pro

gn_run.depends = pdfcore_generator
pdfcore.depends = gn_run

SUBDIRS += \
    pdfcore_generator \
    gn_run \
    pdfcore


