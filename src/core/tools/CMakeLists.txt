
##
#  WEBENGINECORE DICT CONVERT TOOL
##

if(QT_FEATURE_webengine_spellchecker)
    qt_get_tool_target_name(dict_target_name qwebengine_convert_dict)
    qt_internal_add_tool(${dict_target_name}
        TARGET_DESCRIPTION "QtWebEngine Dictionary Conversion Tool"
        INSTALL_DIR ${INSTALL_LIBEXECDIR}
        TOOLS_TARGET WebEngineCore
        SOURCES main.cpp
    )
    qt_skip_warnings_are_errors(${dict_target_name})
    make_install_only(${dict_target_name})
    add_dependencies(${dict_target_name} WebEngineCore)
    qt_internal_extend_target(${dict_target_name} CONDITION WIN32
        DEFINES WIN32_LEAN_AND_MEAN NOMINMAX
    )
endif()

