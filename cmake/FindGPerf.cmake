find_program(GPerf_EXECUTABLE NAMES gperf)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GPerf
    FOUND_VAR
        GPerf_FOUND
    REQUIRED_VARS
        GPerf_EXECUTABLE
)

if(GPerf_FOUND AND NOT TARGET GPerf::GPerf)
    add_executable(GPerf::GPerf IMPORTED)
    set_target_properties(GPerf::GPerf PROPERTIES IMPORTED_LOCATION "${GPerf_EXECUTABLE}")
endif()

include(FeatureSummary)
set_package_properties(GPerf PROPERTIES
    URL "https://www.gnu.org/software/gperf/"
    DESCRIPTION "Perfect hash function generator"
)
