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

FIND_LIBRARY(ZKS_LIB_D NAMES zks_d libzks_d PATHS 
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
FIND_LIBRARY(ZKS_LIB NAMES zks libzks PATHS 
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
string(CONCAT ZKS_LIBRARY "debug;" ${ZKS_LIB_D} ";general;" ${ZKS_LIB} )
