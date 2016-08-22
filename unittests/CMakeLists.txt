find_package (Boost COMPONENTS unit_test_framework OPTIONAL)

IF( Boost_FOUND )
    include_directories ("${PROJECT_SOURCE_DIR}/source" ${Boost_INCLUDE_DIRS})

    add_definitions (-DBOOST_TEST_DYN_LINK)

    add_executable(unittests EXCLUDE_FROM_ALL test.cpp)
    target_link_libraries(unittests mimeapps ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
ENDIF()
