qt_feature("webenginequick-qml" PRIVATE
    LABEL "Support Qt WebEngine Qml"
    PURPOSE "Provides WebEngine Qml support."
)
qt_feature("webenginequick-ui-delegates" PRIVATE
    SECTION "WebEngine"
    LABEL "UI Delegates"
)
qt_feature("webenginequick-testsupport" PRIVATE
    LABEL "Test Support"
    AUTODETECT FALSE
)
qt_configure_add_summary_section(NAME "Qt WebEngineQuick")
qt_configure_add_summary_entry(ARGS "webenginequick-qml")
qt_configure_add_summary_entry(ARGS "webenginequick-ui-delegates")
qt_configure_add_summary_entry(ARGS "webenginequick-testsupport")
qt_configure_end_summary_section()
