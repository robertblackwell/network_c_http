cmake_minimum_required(VERSION 3.16)

##
##
## That is - knows nothing about installing this project
##

##
## Project 
## name  version
##
project(C_HTTP VERSION 0.1.0 LANGUAGES C)

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
option(C_HTTP_BuildTests "Build the tests when enabled" ON )
option(C_HTTP_Install_Targets "Installs targets as well as files if set" OFF)
option(C_HTTP_Package "Install as a CMake package - not implemented" OFF)
option(C_HTTP_Verbose "Print a lot of diagnostic stuff" ON)
option(C_HTTP_DebugBuild "Perform build as type Debug" ON)
option(C_HTTP_BuildDocs "Build doxygen documentation" ON)
option(C_HTTP_SyncServerUseQueue, "Determines whether the sync_server app uses a queue or shares a listening sockets between threads")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
##=============================================================================
# project C compile options
##=============================================================================

# -fms-extensions enables a Microsoft extension that allows merging anonymous struct into a struct
# used extensively in the runloop type definitions. Its a form of inheritence
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fms-extensions")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -pthread -fvisibility=hidden")

#_GNU_SOURCE
#	because we use asprintf()
set (CHTTP_C_FLAGS "${CHTTP_C_FLAGS} -D_GNU_SOURCE")
set (CHTTP_CXX_FLAGS "${CHTTP_CXX_FLAGS} -D_GNU_SOURCE")

#RBL_TAG_CHECK_ON
#	puts a validation tag at the start of every struct and checks it on every access
set (CHTTP_C_FLAGS "${CHTTP_C_FLAGS} -DCH_TAG_CHECK_ON")
set (CHTTP_CXX_FLAGS "${CHTTP_CXX_FLAGS} -DCH_TAG_CHECK_ON")

#CH_SYNC_WORKER_QUEUE
#	the work threads in the sync_server uses an interthread queue for getting accept()'d sockets
#   to the worker threads.
#   When disabled uses a connom listener socket for all threads and each thread does its own accept
#set (CHTTP_C_FLAGS "${CHTTP_C_FLAGS}  -DSYNC_WORKER_QUEUEX")
#set (CHTTP_CXX_FLAGS "${CHTTP_CXX_FLAGS}  -DSYNC_WORKER_QUEUEX")

#CH_SCRIBBLE_ONFREE
#		Writes a pattern over the memory occupied by a struct when free'd
#		paranoid - trying to provoke errors
set (CHTTP_C_FLAGS "${CHTTP_CXX_FLAGS} -DCH_SCRIBBLE")
set (CHTTP_CXX_FLAGS "${CHTTP_CXX_FLAGS} -DCH_SCRIBBLE")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LINUX_FLAG on)
	set(CHTTP_C_FLAGS "${CHTTP_C_FLAGS} -DLINUX_FLAG")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(APPLE_FLAG on)
	set(CHTTP_C_FLAGS "${CHTTP_C_FLAGS} -DAPPLE_FLAG")
endif()

#CH_ASYNC_SINGLE_THREAD
# Forces the async server to run as a single threaded server. Hopefully a temporary
# restriction while I figure out shared listening sockets
# If disabled async_server will be multi threaded using a shared listening socket
set (CHTTP_C_FLAGS "${CHTTP_C_FLAGS} -DCH_ASYNC_SINGLE_THREAD")

#enable logging everywhere
#set (CHTTP_C_FLAGS "${CHTTP_C_FLAGS} -DRBL_LOG_ENABLED -DRBL_LOG_ALLOW_GLOBAL")
#set (CHTTP_CXX_FLAGS "${CHTTP_C_FLAGS} -DRBL_LOG_ENABLED -DRBL_LOG_ALLOW_GLOBAL")


message("CHTTP_C_FLAGS ${CHTTP_C_FLAGS}")
## ============================================================================
## The stuff below - I am not sure about ======================================
##

# this is a fix for a problem with boost libraries see https://github.com/Microsoft/vcpkg/issues/4497
# also a hack and breaks "best practices" as per Damiel Pfeiffer's presentation https://www.youtube.com/watch?v=rLopVhns4Zs&feature=youtu.be
set(other_flags "${other_flags} -frtti -fvisibility-inlines-hidden")
set(other_flags "${other_flags} -fvisibility=hidden -D_GNU_SOURCE -DTYPE_CHECK_ON -D_GNU_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${other_flags} -pthread")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11") # this caused a problem with time.h
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fms-extensions")
# -D_GNU_SOURCE -DTYPE_CHECK_ON -DSYNC_WORKER_QUEUEX -DCH_TAG_CHECK_ON")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CHTTP_C_FLAGS}")

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

if (C_HTTP_DebugBuild)
	set(CMAKE_BUILD_TYPE Debug)
endif()
# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
if(DEFINED CMAKE_DL_LIBS)
    set(C_HTTP_DL_LIB_SO libdl.so)
endif()

set(C_HTTP_PROJECT_DIR       ${C_HTTP_SOURCE_DIR})
## location of project header and source files
set(C_HTTP_SOURCE_NAME       src)
set(C_HTTP_INCLUDE_NAME      src)
set(C_HTTP_TOOLS_NAME        tools)
set(C_HTTP_LIB_NAME          c_http_library)
set(C_HTTP_SOURCE_DIR        ${C_HTTP_PROJECT_DIR}/${C_HTTP_SOURCE_NAME})
set(C_HTTP_INCLUDE_DIR       ${C_HTTP_PROJECT_DIR}/${C_HTTP_INCLUDE_NAME})
set(C_HTTP_TOOLS_DIR         ${C_HTTP_PROJECT_DIR}/${C_HTTP_TOOLS_NAME})
## location of other/3rd party headers, src and libs
set(C_HTTP_VENDOR_NAME		 vendor)
set(C_HTTP_VENDOR_DIR        ${C_HTTP_PROJECT_DIR}/${C_HTTP_VENDOR_NAME})
set(C_HTTP_VENDOR_INCLUDEDIR ${C_HTTP_VENDOR_DIR}/include)
set(C_HTTP_VENDOR_LIBDIR     ${C_HTTP_VENDOR_DIR}/lib)
set(C_HTTP_VENDOR_SRCDIR     ${C_HTTP_VENDOR_DIR}/src)

if (${C_HTTP_BuildDocs} AND ${DOXYGEN_FOUND})
	set(C_HTTP_BUILD_DOCS TRUE)
else()
	set(C_HTTP_BUILD_DOCS FALSE)
endif()

## used by all compile steps to find headers
set(C_HTTP_INCLUDE_PATHS 
	${C_HTTP_PROJECT_DIR}
	${C_HTTP_INCLUDE_DIR}
	${C_HTTP_SOURCE_DIR}
	${C_HTTP_VENDOR_INCLUDEDIR}
	${C_HTTP_VENDOR_SRCDIR}
)
set(C_HTTP_LINK_LIBRARIES 
	${C_HTTP_LIB_NAME} 
	Threads::Threads
	m
	${CMAKE_DL_LIBS}
	# ${C_HTTP_VENDOR_LIBDIR}/libcert_library.a
	# ${C_HTTP_VENDOR_LIBDIR}/libboost_filesystem.a 
	# ${C_HTTP_VENDOR_LIBDIR}/libboost_system.a
	# ${C_HTTP_VENDOR_LIBDIR}/libboost_program_options.a
	# ${C_HTTP_VENDOR_LIBDIR}/libssl.a 
	# ${C_HTTP_VENDOR_LIBDIR}/libcrypto.a 
	${CMAKE_DL_LIBS}
	${DL_LIB_SO}
	${C_HTTP_LIB_NAME}
	)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY XCODE_GENERATE_SCHEME ON)

set(CHLOG_GLOBAL On)


if (C_HTTP_Verbose)
	message("Project Name                      ${CMAKE_PROJECT_NAME}")
	message("C Compiler                        ${CMAKE_C_COMPILER}")
	message("Build Type                        ${CMAKE_BUILD_TYPE}")
	message("CMAKE_CXX_STANDARD                ${CMAKE_CXX_STANDARD}")
	message("CMAKE_C_FLAGS                     ${CMAKE_C_FLAGS}")
	message("CMAKE_CXX_FLAGS                   ${CMAKE_CXX_FLAGS}")
	message("CMAKE_DL_LIBS                     ${CMAKE_DL_LIBS}")
	message("C_HTTP_PROJECT_DIR                ${C_HTTP_PROJECT_DIR}")
	message("C_HTTP_SOURCE_DIR                 ${C_HTTP_SOURCE_DIR}")
	message("C_HTTP_INCLUDE_DIR                ${C_HTTP_INCLUDE_DIR}")
	message("C_HTTP_VENDOR_NAME                ${C_HTTP_VENDOR_NAME}")
	message("C_HTTP_VENOR_DIR                  ${C_HTTP_VENDOR_DIR}")
	message("C_HTTP_VENDOR_INCLUDEDIR          ${C_HTTP_VENDOR_INCLUDEDIR}")
	message("C_HTTP_VENDOR_LIBDIR              ${C_HTTP_VENDOR_LIBDIR}")
	message("C_HTTP_VENDOR_SRCDIR              ${C_HTTP_VENDOR_SRCDIR}")
	message("C_HTTP_VENDOR_SRCDIR              ${C_HTTP_VENDOR_SRCDIR}")
	message("C_HTTP_DL_LIB_SO                  ${C_HTTP_DL_LIB_SO}")
	message("C_HTTP_INCLUDE_PATHS              ${C_HTTP_INCLUDE_PATHS}")
	message("C_HTTP_LINK_LIBRARIES             ${C_HTTP_LINK_LIBRARIES}")
	message("C_HTTP_BuildDocsS                 ${C_HTTP_BuildDocs}")
	message("DOXYGEN_FOUND                     ${DOXYGEN_FOUND}")
	message("C_HTTP_BUILD_DOCS                 ${C_HTTP_BUILD_DOCS}")
	message("CMAKE_INSTALL_PREFIX              ${CMAKE_INSTALL_PREFIX}")
	message("CMAKE_INSTALL_FULL_INCLUDEDIR     ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
	message("CMAKE_INSTALL_INCLUDEDIR          ${CMAKE_INSTALL_INCLUDEDIR}")
	message("C_HTTP_LIB_NAME                   ${C_HTTP_LIB_NAME}")
endif()

include(CTest) 
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
#add_subdirectory(sync_server_app)
#add_subdirectory(verifier)
#add_subdirectory(async-server-app)
#add_subdirectory(apps_demo)
#add_subdirectory(apps_http)

# Doxygen

# look for Doxygen package
find_package(Doxygen)

if (DOXYGEN_FOUND)
	# set input and output files
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs_doxygen/Doxyfile.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)

	# request to configure the file
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	message("Doxygen build started")

	# Note: do not put "ALL" - this builds docs together with application EVERY TIME!
	add_custom_target( docs
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM )
else (DOXYGEN_FOUND)
	message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)