cmake_minimum_required (VERSION 2.8.5) 
project(fw)

add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-std=c++1y>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-g>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-O2>)

set( CMAKE_VERBOSE_MAKEFILE on )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	string(CONCAT CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " -framework CoreFoundation -framework IOKit")
endif()

######################### libzks ##################################

FIND_PATH(ZKS_INCLUDE_DIR
  libzks/libzks.h
  PATHS
  "$ENV{HOME}/local/include/"
  /usr/include/
  /usr/local/include/
  #MSVC
  "$ENV{LIB_DIR}/include"
  "$ENV{UserProfile}/local/include/"
  $ENV{ZKS_INCLUDE_PATH}
  #mingw
  c:/msys/local/include
  )
FIND_LIBRARY(ZKS_LIBRARY NAMES zks libzks zks_d libzks_d PATHS 
  /usr/local/lib 
  /usr/lib 
  "$ENV{HOME}/local/lib"
  #MSVC
  "$ENV{LIB_DIR}/lib"
  "$ENV{UserProfile}/local/lib"
  $ENV{ZKS_LIB_PATH}
  #mingw
  c:/msys/local/lib
  )

IF (ZKS_INCLUDE_DIR AND ZKS_LIBRARY)
   SET(ZKS_FOUND TRUE)
ENDIF (ZKS_INCLUDE_DIR AND ZKS_LIBRARY)

IF (ZKS_FOUND)
   IF (NOT ZKS_FIND_QUIETLY)
      MESSAGE(STATUS "Found libzks : ${ZKS_LIBRARY}")
      MESSAGE(STATUS "Found libzks/libzks.h : ${ZKS_INCLUDE_DIR}")
   ENDIF (NOT ZKS_FIND_QUIETLY)
ELSE (ZKS_FOUND)
      MESSAGE(FATAL_ERROR "Could not find LIBZKS")
ENDIF (ZKS_FOUND)

######################### asio ##################################

FIND_PATH(ASIO_INCLUDE_DIR
  asio.hpp
  PATHS
  "$ENV{HOME}/local/include/"
  /usr/include/
  /usr/local/include/
  #MSVC
  "$ENV{UserProfile}/local/include/"
  "$ENV{LIB_DIR}/include"
  "$ENV{ASIO_INCLUDE_PATH}"
  #mingw
  c:/msys/local/include
  )

IF (ASIO_INCLUDE_DIR)
   SET(ASIO_FOUND TRUE)
ENDIF (ASIO_INCLUDE_DIR)

IF (ASIO_FOUND)
   IF (NOT ASIO_FIND_QUIETLY)
      MESSAGE(STATUS "Found asio.hpp : ${ASIO_INCLUDE_DIR}")
   ENDIF (NOT ASIO_FIND_QUIETLY)
ELSE (ASIO_FOUND)
      MESSAGE(FATAL_ERROR "Could not find asio")
ENDIF (ASIO_FOUND)

######################### protobuff ##################################

find_package(Protobuf REQUIRED)


IF (PROTOBUF_FOUND)
   IF (NOT PROTOBUF_FIND_QUIETLY)
      MESSAGE(STATUS "Found protobuf : ${PROTOBUF_LIBRARIES}")
      MESSAGE(STATUS "Found google/protobuf/message.h : ${PROTOBUF_INCLUDE_DIRS}")
	  MESSAGE(STATUS "Found protoc : ${PROTOBUF_PROTOC_EXECUTABLE}")
	  MESSAGE(STATUS "Found libs : ${PROTOBUF_LIBRARY}")
   ENDIF (NOT PROTOBUF_FIND_QUIETLY)
ELSE (PROTOBUF_FOUND)
	  MESSAGE(STATUS "include: ${PROTOBUF_INCLUDE_DIRS}")
	  MESSAGE(STATUS "lib: ${PROTOBUF_LIBRARIES}")
      MESSAGE(FATAL_ERROR "Could not find protobuf")
ENDIF (PROTOBUF_FOUND)

######################### libfcgi ##################################

FIND_PATH(FCGI_INCLUDE_DIR
  fcgio.h
  PATHS
  $ENV{HOME}/local/include
  /usr/include
  /usr/local/include
  /usr/include/fastcgi
  #MSVC
  "$ENV{UserProfile}/local/include/"
  "$ENV{UserProfile}/local/include/fastcgi/"
  "$ENV{LIB_DIR}/include/fastcgi/"
  $ENV{FCGI_INCLUDE_PATH}
  #mingw
  c:/msys/local/include
  )
FIND_LIBRARY(FCGI_LIBRARY NAMES fcgi libfcgi PATHS 
  /usr/local/lib 
  /usr/lib
  $ENV{HOME}/local/lib
  #MSVC
  "$ENV{UserProfile}/local/lib/"
  "$ENV{LIB_DIR}/lib/"
  $ENV{FCGI_LIBRARY_PATH}
  #mingw
  c:/msys/local/lib
  )
FIND_LIBRARY(FCGIXX_LIBRARY NAMES fcgi++ libfcgi++ PATHS 
  /usr/local/lib 
  /usr/lib
  $ENV{HOME}/local/lib
  #MSVC
  "$ENV{UserProfile}/local/lib"
  "$ENV{LIB_DIR}/lib"
  $ENV{FCGI_LIBRARY_PATH}
  #mingw
  c:/msys/local/lib
  )
  
IF (FCGI_INCLUDE_DIR AND FCGI_LIBRARY)
   SET(FCGI_FOUND TRUE)
ENDIF (FCGI_INCLUDE_DIR AND FCGI_LIBRARY)

IF (FCGI_FOUND)
   IF (NOT FCGI_FIND_QUIETLY)
      MESSAGE(STATUS "Found FCGI lib: ${FCGI_LIBRARY}")
      MESSAGE(STATUS "Found FCGI include: ${FCGI_INCLUDE_DIR}")
      MESSAGE(STATUS "Found FCGI++ lib: ${FCGIXX_LIBRARY}")
   ENDIF (NOT FCGI_FIND_QUIETLY)
ELSE (FCGI_FOUND)
	  MESSAGE(STATUS "include: ${FCGI_INCLUDE_DIR}")
	  MESSAGE(STATUS "lib: ${FCGI_LIBRARY_PATH}")
	  MESSAGE(STATUS "lib++: ${FCGIXX_LIBRARY}")
	  MESSAGE(FATAL_ERROR "Could not find FCGI")
ENDIF (FCGI_FOUND)

######################### thread ##################################

FIND_PACKAGE ( Threads REQUIRED )

add_subdirectory (simfcgi)
add_subdirectory (simproto)
