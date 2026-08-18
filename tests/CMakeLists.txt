qt_build_tests()

if(QT_FEATURE_test_fuzz)
    add_subdirectory(fuzz)
endif()

