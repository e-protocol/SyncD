# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.8)
project ("syncD")
set(CMAKE_CXX_STANDARD 17)

# Включите подпроекты.
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
endif()

include_directories("src")
file(GLOB_RECURSE PROJECT_SOURCES "src/*.h" "src/*.cpp" "src/*.rc")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
