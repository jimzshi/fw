cmake_minimum_required (VERSION 2.6) 
project(fw)

add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-std=c++1y>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-g>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-O2>)

#set( CMAKE_VERBOSE_MAKEFILE on )

FIND_PATH(ZKS_INCLUDE_DIR
  libzks/libzks.h
  PATHS
  "$ENV{HOME}/local/include/"
  /usr/include/
  /usr/local/include/
  #MSVC
  "$ENV{LIB_DIR}/include"
  $ENV{INCLUDE}
  #mingw
  c:/msys/local/include
  )
FIND_LIBRARY(ZKS_LIBRARY NAMES zks libzks PATHS 
  /usr/local/lib 
  /usr/lib 
  "$ENV{HOME}/local/lib"
  #MSVC
  "$ENV{LIB_DIR}/lib"
  $ENV{LIB}
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

FIND_PATH(FCGI_INCLUDE_DIR
  fcgio.h
  PATHS
  /usr/include
  /usr/local/include
  /usr/include/fastcgi
  #MSVC
  "$ENV{LIB_DIR}/include"
  $ENV{INCLUDE}
  #mingw
  c:/msys/local/include
  )
FIND_LIBRARY(FCGI_LIBRARY NAMES fcgi libfcgi PATHS 
  /usr/local/lib 
  /usr/lib
  $ENV{HOME}/local
  #MSVC
  "$ENV{LIB_DIR}/lib"
  $ENV{LIB}
  #mingw
  c:/msys/local/lib
  )
FIND_LIBRARY(FCGIXX_LIBRARY NAMES fcgi++ libfcgi++ PATHS 
  /usr/local/lib 
  /usr/lib
  $ENV{HOME}/local
  #MSVC
  "$ENV{LIB_DIR}/lib"
  $ENV{LIB}
  #mingw
  c:/msys/local/lib
  )
  
IF (FCGI_INCLUDE_DIR AND FCGI_LIBRARY AND FCGIXX_LIBRARY)
   SET(FCGI_FOUND TRUE)
ENDIF (FCGI_INCLUDE_DIR AND FCGI_LIBRARY AND FCGIXX_LIBRARY)

IF (FCGI_FOUND)
   IF (NOT FCGI_FIND_QUIETLY)
      MESSAGE(STATUS "Found FCGI lib: ${FCGI_LIBRARY}")
      MESSAGE(STATUS "Found FCGI include: ${FCGI_INCLUDE_DIR}")
      MESSAGE(STATUS "Found FCGI++ lib: ${FCGIXX_LIBRARY}")
   ENDIF (NOT FCGI_FIND_QUIETLY)
ELSE (FCGI_FOUND)
      MESSAGE(FATAL_ERROR "Could not find FCGI")
ENDIF (FCGI_FOUND)

FIND_PACKAGE ( Threads REQUIRED )

add_subdirectory (simfcgi)