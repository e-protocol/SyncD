cmake_minimum_required (VERSION 3.13)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>" CACHE INTERNAL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
string(APPEND CMAKE_CXX_FLAGS_RELEASE " /MT")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_BINARY_DIR}/build>")


project ("syncd")
add_subdirectory ("src")