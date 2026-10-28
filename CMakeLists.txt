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
fetchcontent_declare(nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
  URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
  SYSTEM)

set(JSON_MultipleHeaders OFF)

include(CTest)

fetchcontent_makeavailable(webview nlohmann_json)

# Pure chess logic: no webview, no JSON, so tests can link on its own.
add_library(pcr_core STATIC
  src/cpp/chess/position.cpp
  src/cpp/chess/perft.cpp
  src/cpp/chess/uci.cpp)
target_include_directories(pcr_core PUBLIC src/cpp)

add_executable(pcr src/cpp/main.cpp)
target_link_libraries(pcr PRIVATE pcr_core webview::core nlohmann_json::nlohmann_json)
target_compile_definitions(pcr
  PRIVATE PCR_WEB_DIST="${PROJECT_SOURCE_DIR}/build/web")

if(BUILD_TESTING)
  fetchcontent_declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG 08092139210881f01a34e60c5f2fd6afa0ea4024 # v3.16.1
    SYSTEM)
  fetchcontent_makeavailable(Catch2)

  add_executable(pcr_tests tests/cpp/types_tests.cpp tests/cpp/position_tests.cpp
  tests/cpp/make_move_tests.cpp tests/cpp/attack_tests.cpp tests/cpp/movegen_tests.cpp
  tests/cpp/perft_tests.cpp tests/cpp/uci_tests.cpp)
  target_link_libraries(pcr_tests PRIVATE pcr_core Catch2::Catch2WithMain)

  # Catch2 shipa a CMake module that registers each TEST_CASE with CTest.
  list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
  include(Catch)
  catch_discover_tests(pcr_tests)
endif()
