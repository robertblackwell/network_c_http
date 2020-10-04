cmake_minimum_required(VERSION 3.16)

##
## NOTE/WARNING - this file will not work if trying to build anywhere other 
## than inside the marvin++ repo
##
## That is - knows nothing about installing this project
##

##
## Project 
## name  version
##
project(C_EG VERSION 0.1.0 LANGUAGES C)

##
## Includes
##
##

##
## find packages
##
##
find_package(Threads REQUIRED)
find_program(BASH_PROG bash)
message("BASH_PROG-NOTFOUND       ${BASH_PROG-NOTFOUND}")
message("BASH_PROG                ${BASH_PROG}")
find_package(Doxygen)
message("Doxygen                  ${DOXYGEN_FOUND}")
message("Doxygen Executable       ${DOXYGEN_EXECUTABLE}")
message("Doxygen Working dir      ${CMAKE_CURRENT_SOURCE_DIR}/docs")

## not sure any of these work for the moment so turn them OFF
if(OFF)
find_package(boost)
find_package(openssl)
find_package(trog)
find_package(x509_certificate_library)
endif()

##
## options
##
option(C_EG_BuildTests "Build the tests when enabled" ON )
option(C_EG_Install_Targets "Installs targets as well as files if set" OFF)
option(C_EG_Package "Install as a CMake package - not implemented" OFF)
option(C_EG_Verbose "Print a lot of diagnostic stuff" ON)
option(C_EG_DebugBuild "Perform build as type Debug" ON)
option(C_EG_BuildDocs "Build doxygen documentation" ON)



## ============================================================================
## The stuff below - I am not sure about ======================================
##

# this is a fix for a problem with boost libraries see https://github.com/Microsoft/vcpkg/issues/4497
# also a hack and breaks "best practices" as per Damiel Pfeiffer's presentation https://www.youtube.com/watch?v=rLopVhns4Zs&feature=youtu.be
set(other_flags "${other_flags} -frtti -fvisibility-inlines-hidden")
set(other_flags "${other_flags} -fvisibility=hidden -D_GNU_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${other_flags} -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CTEST_CUSTOM_PRE_TEST "echo THIS IS PRE TEST")
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Let's nicely support folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# allow scheme porperties in xcode - particularly environment variables
set_property(GLOBAL PROPERTY XCODE_GENERATE_SCHEME ON)
set(XCODE_GENERATE_SCHEME ON)
##
## The stuff below - I am not sure about ====================================
## ==========================================================================

##
##
## Configuration
##
include (GNUInstallDirs)

if (C_EG_DebugBuild)
	set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
if(DEFINED CMAKE_DL_LIBS)
    set(C_EG_DL_LIB_SO libdl.so)
endif()

set(C_EG_PROJECT_DIR       ${C_EG_SOURCE_DIR})
## location of project header and source files
set(C_EG_SOURCE_NAME       c_eg)
set(C_EG_INCLUDE_NAME      c_eg)
set(C_EG_TOOLS_NAME        tools)
set(C_EG_LIB_NAME          c_eg_library)
set(C_EG_SOURCE_DIR        ${C_EG_PROJECT_DIR}/${C_EG_SOURCE_NAME})
set(C_EG_INCLUDE_DIR       ${C_EG_PROJECT_DIR}/${C_EG_INCLUDE_NAME})
set(C_EG_TOOLS_DIR         ${C_EG_PROJECT_DIR}/${C_EG_TOOLS_NAME})
## location of other/3rd party headers, src and libs
set(C_EG_VENDOR_NAME		 vendor)
set(C_EG_VENDOR_DIR        ${C_EG_PROJECT_DIR}/${C_EG_VENDOR_NAME})
set(C_EG_VENDOR_INCLUDEDIR ${C_EG_VENDOR_DIR}/include)
set(C_EG_VENDOR_LIBDIR     ${C_EG_VENDOR_DIR}/lib)
set(C_EG_VENDOR_SRCDIR     ${C_EG_VENDOR_DIR}/src)

if (${C_EG_BuildDocs} AND ${DOXYGEN_FOUND})
	set(C_EG_BUILD_DOCS TRUE)
else()
	set(C_EG_BUILD_DOCS FALSE)
endif()

## used by all compile steps to find headers
set(C_EG_INCLUDE_PATHS 
	${C_EG_PROJECT_DIR}
	# ${C_EG_INCLUDE_DIR}
	# ${C_EG_SOURCE_DIR}
	${C_EG_VENDOR_INCLUDEDIR}
	${C_EG_VENDOR_SRCDIR}
)
set(C_EG_LINK_LIBRARIES 
	${C_EG_LIB_NAME} 
	Threads::Threads
	m
	${CMAKE_DL_LIBS}
	# ${C_EG_VENDOR_LIBDIR}/libcert_library.a
	# ${C_EG_VENDOR_LIBDIR}/libboost_filesystem.a 
	# ${C_EG_VENDOR_LIBDIR}/libboost_system.a
	# ${C_EG_VENDOR_LIBDIR}/libboost_program_options.a
	# ${C_EG_VENDOR_LIBDIR}/libssl.a 
	# ${C_EG_VENDOR_LIBDIR}/libcrypto.a 
	${CMAKE_DL_LIBS}
	${DL_LIB_SO}
	)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY XCODE_GENERATE_SCHEME ON)

if (C_EG_Verbose)
	message("Project Name                      ${CMAKE_PROJECT_NAME}")
	message("Build Type                        ${CMAKE_BUILD_TYPE}")
	message("CMAKE_CXX_STANDARD                ${CMAKE_CXX_STANDARD}")
	message("CMAKE_DL_LIBS                     ${CMAKE_DL_LIBS}")
	message("C_EG_PROJECT_DIR                  ${C_EG_PROJECT_DIR}")
	message("C_EG_SOURCE_DIR                   ${C_EG_SOURCE_DIR}")
	message("C_EG_INCLUDE_DIR                  ${C_EG_INCLUDE_DIR}")
	message("C_EG_VENDOR_NAME                  ${C_EG_VENDOR_NAME}")
	message("C_EG_VENOR_DIR                    ${C_EG_VENDOR_DIR}")
	message("C_EG_VENDOR_INCLUDEDIR            ${C_EG_VENDOR_INCLUDEDIR}")
	message("C_EG_VENDOR_LIBDIR                ${C_EG_VENDOR_LIBDIR}")
	message("C_EG_VENDOR_SRCDIR                ${C_EG_VENDOR_SRCDIR}")
	message("C_EG_VENDOR_SRCDIR                ${C_EG_VENDOR_SRCDIR}")
	message("C_EG_DL_LIB_SO                    ${C_EG_DL_LIB_SO}")
	message("C_EG_INCLUDE_PATHS                ${C_EG_INCLUDE_PATHS}")
	message("C_EG_LINK_LIBRARIES               ${C_EG_LINK_LIBRARIES}")
	message("C_EG_BuildDocsS                   ${C_EG_BuildDocs}")
	message("DOXYGEN_FOUND                     ${DOXYGEN_FOUND}")
	message("C_EG_BUILD_DOCS                   ${C_EG_BUILD_DOCS}")
	message("CMAKE_INSTALL_PREFIX              ${CMAKE_INSTALL_PREFIX}")
	message("CMAKE_INSTALL_FULL_INCLUDEDIR     ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
	message("CMAKE_INSTALL_INCLUDEDIR          ${CMAKE_INSTALL_INCLUDEDIR}")
	message("C_EG_LIB_NAME                     ${C_EG_LIB_NAME}")
endif()

include(CTest) 
enable_testing()

add_subdirectory(c_eg)
add_subdirectory(tests)
add_subdirectory(app)
add_subdirectory(verifier)

