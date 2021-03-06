# Vega - Copyright (C) 2012-2013 Alneos
#
# See the LICENSE.txt file for license information. Please report all
# bugs and problems to <info@alneos.fr>.
# Initial version from cmake config in : http://geuz.org/gmsh

cmake_minimum_required(VERSION 2.8.2 FATAL_ERROR)

set(VEGA_MAJOR_VERSION 1)
set(VEGA_MINOR_VERSION 8)
set(VEGA_PATCH_VERSION 5)
    
# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/cmake_files)

SET(CMAKE_BUILD_TYPE Debug CACHE STRING  
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.")
#make outputs build commands and options
OPTION(CMAKE_VERBOSE_MAKEFILE "make outputs build commands" ON)
project ( vegapp CXX C)
    
IF(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]") 
    message(STATUS "Debug mode selected") 
    option(STATIC_LINKING "Static Linking" OFF)
else(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    message(STATUS "Release mode")
    option(STATIC_LINKING "Static Linking" ON)
ENDIF(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]") 

if(WIN32)
    # Patch for MSVC10 generator, 64 bit
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -DBOOST_ALL_NO_LIB)
    if(STATIC_LINKING)
        #for now under windows don't use static runtime
        #add_compile_options(/MT)
        #set(CMAKE_EXE_LINKER_FLAGS "-pthread -static-libgcc -static-libstdc++ -static" CACHE INTERNAL "Linker flags")
    else(STATIC_LINKING)
        ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
        #add_compile_options(-fPIC)
        #set(CMAKE_EXE_LINKER_FLAGS "-pthread -shared-libgcc -rdynamic -pipe" CACHE INTERNAL "Linker flags")
    endif(STATIC_LINKING)
else(WIN32)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstack-protector-all -rdynamic")
    #SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
    #set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} -Wl,--strip-all")
    add_compile_options(-std=c++11 -Wall -Wextra -Winit-self -Wsuggest-attribute=const -Wconversion -pedantic -pedantic-errors)
    add_compile_options(-Wfloat-equal -Woverlength-strings -fmessage-length=0) # -Wshadow
    if(STATIC_LINKING)
        add_compile_options(-fPIE)
        set(CMAKE_EXE_LINKER_FLAGS "-pthread -static-libgcc -static-libstdc++ -static" CACHE INTERNAL "Linker flags")
    else(STATIC_LINKING)
        add_compile_options(-fPIC)
        set(CMAKE_EXE_LINKER_FLAGS "-pthread -shared-libgcc -rdynamic -pipe" CACHE INTERNAL "Linker flags")
    endif(STATIC_LINKING)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(WIN32)


#set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG>)
# this variable controls the default value of the "ENABLE_XXX" options which are
# normally set to ON (useful if you want to configure a minimal version of Vega:
# e.g. "cmake -DDEFAULT=0 -DENABLE_NASTRAN=1 -DENABLE_SYSTUS=1")
set(DEFAULT ON CACHE INTERNAL "Default value for enabled-by-default options")

option(ENABLE_ASTER "Enable Aster" ${DEFAULT})
option(ENABLE_NASTRAN "Enable Nastran" ${DEFAULT})
option(ENABLE_SYSTUS "Enable Systus" ${DEFAULT})
option(ENABLE_WRAP_PYTHON "Build Python wrappers" OFF)

set(VEGA_EXTRA_VERSION "" CACHE STRING "Vega extra version string")
set(VEGA_VERSION "${VEGA_MAJOR_VERSION}.${VEGA_MINOR_VERSION}")
set(VEGA_VERSION "${VEGA_VERSION}.${VEGA_PATCH_VERSION}${VEGA_EXTRA_VERSION}")
set(VEGA_SHORT_LICENSE "GPL")

execute_process(COMMAND date "+%Y%m%d" OUTPUT_VARIABLE DATE 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND whoami OUTPUT_VARIABLE PACKAGER 
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT DATE)
  set(DATE "unknown")
endif(NOT DATE)
set(VEGA_DATE "${DATE}")

if(NOT HOSTNAME)
  set(HOSTNAME "unknown")
endif(NOT HOSTNAME)
set(VEGA_HOST "${HOSTNAME}")

if(NOT PACKAGER)
  set(PACKAGER "unknown")
endif(NOT PACKAGER)
set(VEGA_PACKAGER "${PACKAGER}")

if(APPLE)
  set(VEGA_OS "MacOSX")
else(APPLE)
  set(VEGA_OS "${CMAKE_SYSTEM_NAME}")
endif(APPLE)

macro(set_config_option VARNAME STRING)
  set(${VARNAME} TRUE)
  list(APPEND CONFIG_OPTIONS ${STRING})
  message(STATUS "Found " ${STRING})
endmacro(set_config_option)

# Before the add_subdirectory command(s)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET (CTEST_BINARY_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#10 hours for tests
set (CTEST_TEST_TIMEOUT 36000 CACHE STRING 
    "Maximum time allowed before CTest will kill the test.") 

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCXXCompilerFlag)

# check if the machine is 64 bits (this is more reliable than using
# CMAKE_SIZEOF_VOID_P, which does not seem to work e.g. on some Suse
# machines)
#check_type_size("void*" SIZEOF_VOID_P) not working at the moments
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set_config_option(HAVE_64BIT_SIZE_T "Have64BitSizeT")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/")
set(MEDFile_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/" CACHE PATH "MEDFile cmake config path")
if(IS_DIRECTORY ${MEDFile_DIR})
    mark_as_advanced(MEDFile_DIR)
endif(IS_DIRECTORY ${MEDFile_DIR})
    
include(MEDFileConfig)

set(Boost_USE_STATIC_LIBS ${STATIC_LINKING})
#set(Boost_USE_MULTITHREADED ON)
IF(NOT WIN32)
    #under windows don't use static runtime because medC hasn't been compiled with 
    #(see commented compile option /MT)
    set(Boost_USE_STATIC_RUNTIME ${STATIC_LINKING})
ENDIF(NOT WIN32)

find_package(Boost 1.57.0 COMPONENTS thread date_time program_options filesystem system regex unit_test_framework REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories ( ${Boost_LIBRARY_DIRS} )
list(APPEND EXTERNAL_LIBRARIES ${Boost_LIBRARIES})

#works only in Linux, in windows always choose the static version
set(MEDFILE_USE_STATIC_LIBS ${STATIC_LINKING})
find_package(MEDFile CONFIG REQUIRED NO_DEFAULT_PATH)
include_directories(SYSTEM ${MEDFILE_INCLUDE_DIRS})
include_directories(SYSTEM ${HDF5_INCLUDE_DIR})
include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})
#link_directories ( ${MEDFILE_LIBRARY_DIR} )
list(APPEND EXTERNAL_LIBRARIES ${MEDFILE_C_LIBRARIES})
#list(APPEND EXTERNAL_LIBRARIES ${HDF5_LIBRARIES})

#necessary to find build_properties.h
include_directories(${CMAKE_BINARY_DIR})

INCLUDE(CTest)
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

IF(NIGHTLY_BUILD MATCHES "TRUE")
   set_config_option(HAVE_LONG_TESTS "LongTests")
ENDIF(NIGHTLY_BUILD MATCHES "TRUE")
OPTION(TESTS_EXEC_SOLVER "Exec solver" ON)

if(ENABLE_ASTER)
  set_config_option(HAVE_ASTER "Aster")
  add_subdirectory(Aster)
endif(ENABLE_ASTER)

if(ENABLE_NASTRAN)
  set_config_option(HAVE_NASTRAN "Nastran")
  add_subdirectory(Nastran)
endif(ENABLE_NASTRAN)

if(ENABLE_SYSTUS)
  set_config_option(HAVE_SYSTUS "Systus")
  add_subdirectory(Systus)
endif(ENABLE_SYSTUS)

add_subdirectory(Abstract)
add_subdirectory(ResultReaders)
add_subdirectory(Commandline)

IF(BUILD_TESTING)
    ENABLE_TESTING()
    add_subdirectory(Test)
ENDIF()

list(SORT CONFIG_OPTIONS)
set(VEGA_CONFIG_OPTIONS "")
foreach(OPT ${CONFIG_OPTIONS})
  set(VEGA_CONFIG_OPTIONS "${VEGA_CONFIG_OPTIONS} ${OPT}")
endforeach(OPT)


if(ENABLE_WRAP_PYTHON)
  find_package(SWIG)
  find_package(PythonLibs)
  if(SWIG_FOUND AND PYTHONLIBS_FOUND)
    message(STATUS "Found SWIG version " ${SWIG_VERSION})
    string(SUBSTRING ${SWIG_VERSION} 0 1 SWIG_MAJOR_VERSION)
    if(SWIG_MAJOR_VERSION EQUAL 1)
      message("WARNING: Python bindings require SWIG >= 2: disabling Python")
    else(SWIG_MAJOR_VERSION EQUAL 1)
      add_subdirectory(wrappers/vegapy)
    endif(SWIG_MAJOR_VERSION EQUAL 1)
  endif(SWIG_FOUND AND PYTHONLIBS_FOUND)
endif(ENABLE_WRAP_PYTHON)

mark_as_advanced(VEGA_EXTRA_VERSION HDF5_DIR Boost_DIR)

##build Doxygen documentation
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

message("")
message(STATUS "Vega ${VEGA_VERSION} has been configured for ${VEGA_OS} with the "  "following options:" ${VEGA_CONFIG_OPTIONS})
message(STATUS "C compiler: " ${CMAKE_C_COMPILER})
message(STATUS "C++ compiler: " ${CMAKE_CXX_COMPILER})

get_property(INC_DIR DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "Include directories: ${INC_DIR}" )
message(STATUS "External libraries: ${EXTERNAL_LIBRARIES}" )
message(STATUS "C compiler flags: " ${CMAKE_C_FLAGS})
message(STATUS "C++ compiler flags: " ${CMAKE_CXX_FLAGS})
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

  message(STATUS "Install prefix: " ${CMAKE_INSTALL_PREFIX})
if(DOXYGEN_FOUND)
  message(STATUS "Doxygen: " ${DOXYGEN_EXECUTABLE})
  message(STATUS "Doxyfile: " ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)  
else(DOXYGEN_FOUND)
  message(STATUS "Doxygen: not found.")
endif(DOXYGEN_FOUND)
if(${STATIC_LINKING})
    message(STATUS "Static linking.")
else(${STATIC_LINKING})
    message(STATUS "Dynamic linking.")
endif(${STATIC_LINKING})
#message("")
#message("Run 'ccmake ${CMAKE_CURRENT_SOURCE_DIR}' to fine-tune the configuration.")
message("")
message("Targets:")
message(" *      doc: generate documentation.")
