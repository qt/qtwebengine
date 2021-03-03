include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
QT_FOR_CONFIG += pdf-private

gn_args += use_nss_certs=false

qtConfig(webengine-qt-png) {
    gn_args += pdfium_use_qt_libpng=true
    gn_args += "pdfium_qt_libpng_includes=\"$$system_path($$QMAKE_INCDIR_LIBPNG)\""
}

qtConfig(webengine-qt-jpeg) {
    gn_args += use_qt_libjpeg=true
    gn_args += "qt_libjpeg_includes=\"$$system_path($$QMAKE_INCDIR_LIBJPEG)\""
}

qtConfig(webengine-qt-harfbuzz) {
    gn_args += use_qt_harfbuzz=true
    gn_args += "qt_harfbuzz_includes=\"$$system_path($$QMAKE_INCDIR_HARFBUZZ)\""
}

qtConfig(webengine-qt-freetype) {
    gn_args += use_qt_freetype=true
    gn_args += "qt_freetype_includes=\"$$system_path($$QMAKE_INCDIR_FREETYPE)\""
}

qtConfig(webengine-qt-zlib) {
    win32 {
      CONFIG(debug, debug|release) {
          qtzlib = Qt5Cored.lib
      } else {
          qtzlib = Qt5Core.lib
      }

    } else { qtzlib = libQt5Core.a
    }
    gn_args += use_qt_zlib = true
    gn_args += "qt_zlib_includes=\["\
               "\"$$system_path($$[QT_INSTALL_HEADERS])\"," \
               "\"$$system_path($$[QT_INSTALL_HEADERS]/QtCore)\"," \
               "\"$$system_path($$[QT_INSTALL_HEADERS]/QtZlib)\"\]"
    gn_args += "qt_zlib=\"$$system_path($$[QT_INSTALL_LIBS]/$$qtzlib)\""
}

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
