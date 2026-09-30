cmake_minimum_required(VERSION 3.16)
project(LockerHub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug)
endif()

file(GLOB_RECURSE LOCKERHUB_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_library(lockerhub STATIC ${LOCKERHUB_SOURCES})
target_include_directories(lockerhub PUBLIC ${CMAKE_SOURCE_DIR}/src)

if(MSVC)
  target_compile_options(lockerhub PRIVATE /W4 /utf-8)
else()
  target_compile_options(lockerhub PRIVATE -Wall -Wextra)
endif()

# Link the MinGW runtime statically so the executables don't depend on
# whichever libstdc++ DLL happens to be first on PATH.
if(MINGW)
  set(STATIC_RUNTIME -static)
endif()

add_executable(lockerhub-cli app/main.cpp)
target_link_libraries(lockerhub-cli PRIVATE lockerhub ${STATIC_RUNTIME})

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE lockerhub ${STATIC_RUNTIME})
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_compile_definitions(unit_tests PRIVATE SAMPLE_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)
