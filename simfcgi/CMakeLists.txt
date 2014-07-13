include_directories (${ZKS_INCLUDE_DIR} ${FCGI_INCLUDE_DIR})

add_executable (simfcgi-mt simfcgi-mt.cpp)
target_link_libraries (simfcgi-mt ${FCGI_LIBRARY} ${ZKS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} c++)

install (TARGETS simfcgi-mt DESTINATION bin)
install (FILES simfcgi.ini DESTINATION etc)
