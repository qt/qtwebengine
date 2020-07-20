include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
QT_FOR_CONFIG += buildtools-private
TEMPLATE = subdirs
pdfcore.file = pdfcore.pro
pdfcore_generator.file = pdfcore_generator.pro
gn_run.file = gn_run.pro
pdfcore_prl_generator.file = pdfcore_prl_generator.pro
gn_run.depends = pdfcore_generator
pdfcore_prl_generator.depends = gn_run
pdfcore.depends = pdfcore_prl_generator
quick.depends = pdfcore

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
        pdfcore_prl_generator \
        pdfcore \
        quick
}

