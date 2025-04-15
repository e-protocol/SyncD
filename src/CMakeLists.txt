file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/ *.h *.cpp *.hpp)
add_executable (${PROJECT_NAME} ${SRC} appicon.rc)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
