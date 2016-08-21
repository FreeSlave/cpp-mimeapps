include_directories ("${PROJECT_SOURCE_DIR}/source")

add_executable(openwith-cli EXCLUDE_FROM_ALL main.cpp)
target_link_libraries(openwith-cli mimeapps)
