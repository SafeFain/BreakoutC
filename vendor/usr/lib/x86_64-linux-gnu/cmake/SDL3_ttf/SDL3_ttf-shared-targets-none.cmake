#----------------------------------------------------------------
# Generated CMake target import file for configuration "None".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDL3_ttf::SDL3_ttf-shared" for configuration "None"
set_property(TARGET SDL3_ttf::SDL3_ttf-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS NONE)
set_target_properties(SDL3_ttf::SDL3_ttf-shared PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_NONE "SDL3::SDL3-shared"
  IMPORTED_LOCATION_NONE "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libSDL3_ttf.so.0.2.2"
  IMPORTED_SONAME_NONE "libSDL3_ttf.so.0"
  )

list(APPEND _cmake_import_check_targets SDL3_ttf::SDL3_ttf-shared )
list(APPEND _cmake_import_check_files_for_SDL3_ttf::SDL3_ttf-shared "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libSDL3_ttf.so.0.2.2" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
