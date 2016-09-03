FIND_PATH(FASTCGI_INCLUDE_DIR
  fastcgi.h
  PATHS
  "$ENV{HOME}/local/include/"
  /usr/include/
  /usr/local/include/
  #MSVC
  "$ENV{LIB_DIR}/include"
  "$ENV{UserProfile}/local/include/"
  $ENV{FASTCGI_INCLUDE_PATH}
  #mingw
  c:/msys/local/include
  )

FIND_LIBRARY(FASTCGI_D_LIB NAMES fcgi_d PATHS 
    /usr/local/lib 
    /usr/lib 
    "$ENV{HOME}/local/lib"
    #MSVC
    "$ENV{UserProfile}/local/lib"
    "$ENV{LIB_DIR}/lib"
    $ENV{ZKS_LIB_PATH}
    #mingw
    c:/msys/local/lib
    )
FIND_LIBRARY(FASTCGI_LIB NAMES fcgi PATHS 
    /usr/local/lib 
    /usr/lib 
    "$ENV{HOME}/local/lib"
    #MSVC
    "$ENV{UserProfile}/local/lib"
    "$ENV{LIB_DIR}/lib"
    $ENV{ZKS_LIB_PATH}
    #mingw
    c:/msys/local/lib
    )
	
FIND_LIBRARY(FASTCGIPP_D_LIB NAMES fcgi++_d PATHS 
    /usr/local/lib 
    /usr/lib 
    "$ENV{HOME}/local/lib"
    #MSVC
    "$ENV{UserProfile}/local/lib"
    "$ENV{LIB_DIR}/lib"
    $ENV{ZKS_LIB_PATH}
    #mingw
    c:/msys/local/lib
    )

FIND_LIBRARY(FASTCGIPP_LIB NAMES fcgi++ PATHS 
    /usr/local/lib 
    /usr/lib 
    "$ENV{HOME}/local/lib"
    #MSVC
    "$ENV{UserProfile}/local/lib"
    "$ENV{LIB_DIR}/lib"
    $ENV{ZKS_LIB_PATH}
    #mingw
    c:/msys/local/lib
    )
	
string(CONCAT FASTCGI_LIBRARY "debug;" ${FASTCGI_D_LIB} ";general;" ${FASTCGI_LIB} ";debug;" ${FASTCGIPP_D_LIB} ";general;" ${FASTCGIPP_LIB})

IF (FASTCGI_INCLUDE_DIR AND FASTCGI_LIBRARY)
   SET(FASTCGI_FOUND TRUE)
ENDIF (FASTCGI_INCLUDE_DIR AND FASTCGI_LIBRARY)