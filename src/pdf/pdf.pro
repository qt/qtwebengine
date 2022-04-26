include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
QT_FOR_CONFIG += buildtools-private
TEMPLATE = subdirs
pdfcore.file = pdfcore.pro
pdfcore_generator.file = pdfcore_generator.pro

pdfcore_lipo.file = pdfcore_lipo.pro
pdfcore_lipo.depends = gn_run

gn_run.file = gn_run.pro
pdfcore_prl_generator.file = pdfcore_prl_generator.pro
gn_run.depends = pdfcore_generator
pdfcore_prl_generator.depends = gn_run
quick.depends = pdfcore

isUniversal() {
   pdfcore.depends += pdfcore_lipo
} else {
   pdfcore.depends += pdfcore_prl_generator
}

!qtConfig(webengine-qtpdf-support):qtConfig(build-qtpdf)::!build_pass {
    !qtwebengine_makeCheckPdfError() {
        errorbuild.commands = @echo $$shell_quote("QtPdf will not be built. $${skipBuildReason}")
    } else {
        errorbuild.commands = @echo $$shell_quote("QtPdf module will not be built for unknown reason, please open a bug report at https://bugreports.qt.io")
    }
    errorbuild.CONFIG = phony
    QMAKE_EXTRA_TARGETS += errorbuild
    first.depends += errorbuild
    QMAKE_EXTRA_TARGETS += first
} else {
    SUBDIRS += \
        pdfcore_generator \
        gn_run \
        pdfcore \
        quick

    isUniversal() {
        SUBDIRS += pdfcore_lipo
    } else {
        SUBDIRS += pdfcore_prl_generator
     }
}

