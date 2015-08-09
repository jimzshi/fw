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

include(libzks.cmake)

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

include(libfcgi.cmake)

IF (FASTCGI_FOUND)
    MESSAGE(STATUS "Found fastcgi.h : ${FASTCGI_INCLUDE_DIR}")
    MESSAGE(STATUS "Found libfcgi.lib : ${FASTCGI_LIBRARY}")
ELSE (FASTCGI_FOUND)
    MESSAGE(FATAL_ERROR "Could not find fastcgi")
ENDIF (FASTCGI_FOUND)

######################### thread ##################################

FIND_PACKAGE ( Threads REQUIRED )

add_subdirectory (simfcgi)
add_subdirectory (simproto)
