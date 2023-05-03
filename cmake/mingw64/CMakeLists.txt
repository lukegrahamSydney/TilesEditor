set(TARGET_NAME "TilesEditor")

set(CMAKE_DEBUG_POSTFIX _d)

set(BIN_DIR "" CACHE STRING "Binary output directory")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/dist)

link_directories(${PROJECT_SOURCE_DIR}/lib)

# Lowercase project name for binaries and packaging
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# Additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# Version number in format X.YY.ZZ
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 VER_X)
list(GET VERSION_LIST 1 VER_Y)
set(VER_Z 0)
set(VER_EXTRA "-beta" CACHE STRING "Extra version")

# Build date Information
string(TIMESTAMP VER_YEAR "%Y")
string(TIMESTAMP VER_MONTH "%m")
string(TIMESTAMP VER_DAY "%d")
string(TIMESTAMP VER_HOUR "%H")
string(TIMESTAMP VER_MINUTE "%M")

set(VER_EXTRA "${VER_EXTRA} (${VER_YEAR}-${VER_MONTH}-${VER_DAY} ${VER_HOUR}:${VER_MINUTE})")


set(VER_FULL "${VER_X}.${VER_Y}.${VER_Z}${VER_EXTRA}")

set(APP_CREDITS "Luke Graham (39ster)")
set(APP_VENDOR "OpenGraal")

STRING(REGEX REPLACE " " "-" VER_CPACK ${VER_FULL})
STRING(REGEX REPLACE "[\(]" "" VER_CPACK ${VER_CPACK})
STRING(REGEX REPLACE "[\)]" "" VER_CPACK ${VER_CPACK})
STRING(REGEX REPLACE "(-[0-9]+:[0-9]+)" "" VER_CPACK ${VER_CPACK})

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR TGZ)
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME_LOWER})
set(CPACK_PACKAGE_VENDOR "OpenGraal Team")
set(CPACK_PACKAGE_CONTACT "opengraal")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${VER_CPACK}")
set(CPACK_PACKAGE_VERSION_MAJOR ${VER_X})
set(CPACK_PACKAGE_VERSION_MINOR ${VER_Y})
set(CPACK_PACKAGE_VERSION_PATCH ${VER_Z}${VER_EXTRA})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/README.md)
set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README.md)
#set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${VER_CPACK}-src")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.bzr/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)

set(INSTALL_DEST .)

if (MINGW)
    target_link_options(${TARGET_NAME} PRIVATE -static -fstack-protector)
    target_link_libraries(${TARGET_NAME} PRIVATE -static-libgcc -static-libstdc++)

    message(STATUS "${TARGET_NAME} Mingw64 setup")


    install(CODE "set(MY_EXE \"${PROJECT_BINARY_DIR}/${TARGET_NAME}.exe\")")

    list(APPEND CMAKE_FIND_DLL_PATH "/usr/x86_64-w64-mingw32/usr/bin")
    message(STATUS "${CMAKE_FIND_DLL_PATH}")

    # Transfer the value of ${MY_DEPENDENCY_PATHS} into the install script
    install(CODE "set(MY_DEPENDENCY_PATHS \"${CMAKE_FIND_DLL_PATH}\")")
    install(CODE [[
        set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "windows+pe")
        set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "objdump")
        set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "./objdump_unix2dos.sh")

        write_file("./objdump_unix2dos.sh" "${CMAKE_OBJDUMP} $@ | unix2dos")
        file(CHMOD "./objdump_unix2dos.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

	  function(install_library_with_deps LIBRARY)
	  	  message("Getting dependencies for ${LIBRARY}")

		file(INSTALL
		  DESTINATION "${CMAKE_INSTALL_PREFIX}/"
		  TYPE SHARED_LIBRARY
		  FOLLOW_SYMLINK_CHAIN
		  FILES "${LIBRARY}"
		)
		file(GET_RUNTIME_DEPENDENCIES
		  LIBRARIES ${LIBRARY}
		  RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
		  UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
		  DIRECTORIES ${MY_DEPENDENCY_PATHS}
		)
		foreach(FILE ${RESOLVED_DEPS})
		  if(NOT IS_SYMLINK ${FILE})
			install_library_with_deps(${FILE})
		  else()
		  	message( "Symlink ${LIBRARY}: ${FILE}")
		  endif()
		endforeach()
		foreach(FILE ${UNRESOLVED_DEPS})
		  message( "Unresolved from ${LIBRARY}: ${FILE}")
		endforeach()
	  endfunction()
	  message("Getting dependencies for ${MY_EXE}")
	  file(GET_RUNTIME_DEPENDENCIES
			EXECUTABLES ${MY_EXE}
		RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
		UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
		    DIRECTORIES ${MY_DEPENDENCY_PATHS}

	  )
	  foreach(FILE ${RESOLVED_DEPS})
		install_library_with_deps(${FILE})
	  endforeach()
	  foreach(FILE ${UNRESOLVED_DEPS})
		message( "Unresolved: ${FILE}")
	  endforeach()
	]])
endif()

#install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DEST})