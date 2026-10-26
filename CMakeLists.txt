cmake_minimum_required(VERSION 3.25)

project(PersonalChessReps
    VERSION 0.1.0
    DESCRIPTION "A local-first chess repertoire trainer"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_DOCS "Create the Doxygen doc target when Doxygen is available" ON)

if(BUILD_DOCS)
  find_package(Doxygen 1.9 QUIET)

  if(Doxygen_FOUND)
    configure_file(
            "${CMAKE_SOURCE_DIR}/Doxyfile.in"
            "${CMAKE_BINARY_DIR}/Doxyfile"
            @ONLY)

    add_custom_target(docs
            COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Generating Doxygen documentation"
            VERBATIM)
  else()
    message(WARNING
            "Doxygen was not found; the 'docs' target is unavailable. "
            "Install it with 'brew install doxygen' and reconfigure CMake.")
  endif()
endif()

include(FetchContent)

# webview builds C shared/static libraries by default; the C++ API is
# header-only, so only its header target is needed.
set(WEBVIEW_BUILD_SHARED_LIBRARY OFF)
set(WEBVIEW_BUILD_STATIC_LIBRARY OFF)

fetchcontent_declare(webview
  GIT_REPOSITORY https://github.com/webview/webview
  GIT_TAG 782c12ccc3e9358c14a42b41baddbf50bfc28614 # 0.12.0
  SYSTEM)
fetchcontent_makeavailable(webview)

add_executable(pcr src/cpp/main.cpp)
target_link_libraries(pcr PRIVATE webview::core)
target_compile_definitions(pcr
  PRIVATE PCR_WEB_DIR="${PROJECT_SOURCE_DIR}/src/web")
