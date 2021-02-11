# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.8)

project(sprite-editor)
set(PROJECT_SOURCE
    main.cpp
    src/mouse.cpp
    src/menu.cpp
    src/editor.cpp
    src/palette.cpp
    src/preview.cpp
    src/file-browser.cpp
    src/control-icons.cpp
    src/keyboard.cpp)
set(PROJECT_DISTRIBS LICENSE README.md)

# Build configuration; approach this with caution!
if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()
find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk)

blit_executable (${PROJECT_NAME} ${PROJECT_SOURCE})
blit_assets_yaml (${PROJECT_NAME} assets.yml)
blit_metadata (${PROJECT_NAME} metadata.yml)
add_custom_target (flash DEPENDS ${PROJECT_NAME}.flash)

# setup release packages
install (FILES ${PROJECT_DISTRIBS} DESTINATION .)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set (CPACK_GENERATOR "ZIP" "TGZ")
include (CPack)
