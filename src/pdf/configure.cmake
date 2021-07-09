qt_feature("pdf-v8" PRIVATE
    LABEL "Support V8"
    PURPOSE "Enables javascript support."
    AUTODETECT false
    CONDITION NOT IOS
)
qt_feature("pdf-xfa" PRIVATE
    LABEL "Support XFA"
    PURPOSE "Enables XFA support."
    CONDITION QT_FEATURE_pdf_v8
)
qt_feature("pdf-xfa-bmp" PRIVATE
    LABEL "Support XFA-BMP"
    PURPOSE "Enables XFA-BMP support."
    CONDITION QT_FEATURE_pdf_xfa
)
qt_feature("pdf-xfa-gif" PRIVATE
    LABEL "Support XFA-GIF"
    PURPOSE "Enables XFA-GIF support."
    CONDITION QT_FEATURE_pdf_xfa
)
qt_feature("pdf-xfa-png" PRIVATE
    LABEL "Support XFA-PNG"
    PURPOSE "Enables XFA-PNG support."
    CONDITION QT_FEATURE_pdf_xfa
)
qt_feature("pdf-xfa-tiff" PRIVATE
    LABEL "Support XFA-TIFF"
    PURPOSE "Enables XFA-TIFF support."
    CONDITION QT_FEATURE_pdf_xfa
)
qt_configure_add_summary_section(NAME "Qt PDF")
qt_configure_add_summary_entry(ARGS "pdf-v8")
qt_configure_add_summary_entry(ARGS "pdf-xfa")
qt_configure_add_summary_entry(ARGS "pdf-xfa-bmp")
qt_configure_add_summary_entry(ARGS "pdf-xfa-gif")
qt_configure_add_summary_entry(ARGS "pdf-xfa-png")
qt_configure_add_summary_entry(ARGS "pdf-xfa-tiff")
qt_configure_end_summary_section()
