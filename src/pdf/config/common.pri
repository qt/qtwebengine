include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
QT_FOR_CONFIG += pdf-private

qtConfig(pdf-v8) {
    gn_args += pdf_enable_v8=true
} else {
    gn_args += pdf_enable_v8=false
}

qtConfig(pdf-xfa) {
    gn_args += pdf_enable_xfa=true
} else {
    gn_args += pdf_enable_xfa=false
}

qtConfig(pdf-xfa-bmp) {
    gn_args += pdf_enable_xfa_bmp=true
} else {
    gn_args += pdf_enable_xfa_bmp=false
}

qtConfig(pdf-xfa-gif) {
    gn_args += pdf_enable_xfa_gif=true
} else {
    gn_args += pdf_enable_xfa_gif=false
}

qtConfig(pdf-xfa-png) {
    gn_args += pdf_enable_xfa_png=true
} else {
    gn_args += pdf_enable_xfa_png=false
}

qtConfig(pdf-xfa-tiff) {
    gn_args += pdf_enable_xfa_tiff=true
} else {
    gn_args += pdf_enable_xfa_tiff=false
}
