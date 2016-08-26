boost_test = find_library('boost_unit_test_framework')
unittest = executable('unittests', 'test.cpp', 
                      cpp_args : '-DBOOST_TEST_DYN_LINK', 
                      include_directories : inc, 
                      link_with : [mimeapps_lib], 
                      dependencies : boost_test)
test('mimeapps test', unittest)
