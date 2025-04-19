#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "hiredis::hiredis_ssl" for configuration "Debug"
set_property(TARGET hiredis::hiredis_ssl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(hiredis::hiredis_ssl PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/hiredis_ssld.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "hiredis::hiredis"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/hiredis_ssld.dll"
  )

list(APPEND _cmake_import_check_targets hiredis::hiredis_ssl )
list(APPEND _cmake_import_check_files_for_hiredis::hiredis_ssl "${_IMPORT_PREFIX}/lib/hiredis_ssld.lib" "${_IMPORT_PREFIX}/bin/hiredis_ssld.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
