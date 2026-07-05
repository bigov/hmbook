#
# При вызове команды "cmake" (под win32) НЕОБХОДИМО указать тип сборки,
# например, для сборки в MinGW GCC указывается -
#
#    cmake -DCMAKE_BUILD_TYPE=Release ../ -G "MinGW Makefiles"
#
# потом:
#    cmake --build .
#
#
#
# https://cmake.org/Wiki/CMake_Useful_Variables
#
# wxWidgets - контроль (linux/msys2) параметров, необходимых для сборки:
#   wx-config --cxxflags --libs
#

cmake_minimum_required(VERSION 4.2.0)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# Данные проекта
project(HMBOOK LANGUAGES CXX)
SET(HMBOOK "hmbook")
#SET(PROJECT_AUTHOR "bigov")
#SET(PROJECT_AUTHOR_EMAIL "bigov@yandex.ru")
#SET(OperatingSystem "MS Windows")
#SET(HMBOOK_VERSION_MAJOR 0)
#SET(HMBOOK_VERSION_MINOR 2)

SET(HMBOOK_DEPS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty" CACHE PATH
  "Shared directory for downloaded third-party sources")

# Базовая GUI библиотека - wxWidgets
include(FetchContent)

SET(HMBOOK_WX_SOURCE_DIR "${HMBOOK_DEPS_SOURCE_DIR}/wxwidgets-src")
SET(HMBOOK_WX_BINARY_DIR "${HMBOOK_DEPS_SOURCE_DIR}/wxwidgets-${CMAKE_BUILD_TYPE}")

if(EXISTS "${HMBOOK_WX_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(
      wxWidgets
      SOURCE_DIR "${HMBOOK_WX_SOURCE_DIR}"
      BINARY_DIR "${HMBOOK_WX_BINARY_DIR}"
    )
else()
    FetchContent_Declare(
      wxWidgets
      GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
      GIT_TAG "v3.3.2"
      SOURCE_DIR "${HMBOOK_WX_SOURCE_DIR}"
      BINARY_DIR "${HMBOOK_WX_BINARY_DIR}"
    )
endif()

message(STATUS "wxWidgets source cache: ${HMBOOK_WX_SOURCE_DIR}")

SET(wxUSE_WEBVIEW "OFF")
SET(wxWidgets_USE_STATIC "OFF")
SET(wxBUILD_SHARED "ON")
SET(wxBUILD_MONOLITHIC "ON")

FetchContent_GetProperties(wxWidgets)
FetchContent_MakeAvailable(wxWidgets)

# Библиотека парсера Markdown - cmark
SET(HMBOOK_CMARK_SOURCE_DIR "${HMBOOK_DEPS_SOURCE_DIR}/cmark-src")
SET(HMBOOK_CMARK_BINARY_DIR "${HMBOOK_DEPS_SOURCE_DIR}/cmark-${CMAKE_BUILD_TYPE}")

if(EXISTS "${HMBOOK_CMARK_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(
      cmark
      SOURCE_DIR "${HMBOOK_CMARK_SOURCE_DIR}"
      BINARY_DIR "${HMBOOK_CMARK_BINARY_DIR}"
    )
else()
    FetchContent_Declare(
      cmark
      GIT_REPOSITORY "https://github.com/commonmark/cmark.git"
      GIT_TAG "0.31.2"
      SOURCE_DIR "${HMBOOK_CMARK_SOURCE_DIR}"
      BINARY_DIR "${HMBOOK_CMARK_BINARY_DIR}"
    )
endif()

message(STATUS "cmark source cache: ${HMBOOK_CMARK_SOURCE_DIR}")

FetchContent_GetProperties(cmark)
FetchContent_MakeAvailable(cmark)

# Ресурсный файл Windows (манифест, DPI и пр.)
enable_language(RC)
configure_file(
    ${CMAKE_SOURCE_DIR}/resources/app.rc.in
    ${CMAKE_BINARY_DIR}/app.rc
    COPYONLY
)
SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -I\"${wxwidgets_SOURCE_DIR}/include\"")

# Сборка приложения
AUX_SOURCE_DIRECTORY("src" SOURCE)
add_executable(${HMBOOK} ${SOURCE} ${CMAKE_BINARY_DIR}/app.rc)
target_include_directories(${HMBOOK} PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
target_compile_definitions(${HMBOOK} PRIVATE WXUSINGDLL __WXMSW__)

# Скрыть консоль
target_link_options(${HMBOOK} PRIVATE -mwindows -Wl,-subsystem,windows)

target_link_libraries(${HMBOOK} PRIVATE wxWidgets::wxWidgets cmark)

# Добавление .dll библиотеки в каталог запуска
SET(BIN_LIB "${HMBOOK_WX_BINARY_DIR}/lib/gcc_x64_dll")
set(WX_DLL_SUFFIX "")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(WX_DLL_SUFFIX "d")
endif()
add_custom_command( TARGET "${HMBOOK}" POST_BUILD
  COMMAND ${CMAKE_COMMAND}
    -E copy "${BIN_LIB}/wxmsw332u${WX_DLL_SUFFIX}_gcc_x64_custom.dll" "${CMAKE_BINARY_DIR}/"
  COMMAND ${CMAKE_COMMAND}
    -E copy_directory "${CMAKE_SOURCE_DIR}/resources/images" "${CMAKE_BINARY_DIR}/images"
)


