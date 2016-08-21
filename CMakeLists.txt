cmake_minimum_required (VERSION 2.6)
project (mimeapps)
add_subdirectory (source) 
add_subdirectory (examples/openwith-cli) 

enable_testing ()
add_subdirectory (unittests)
add_test (NAME Unittests COMMAND unittests)
