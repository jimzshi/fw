include_directories (${ZKS_INCLUDE_DIR} ${FCGI_INCLUDE_DIR})

add_executable (simfcgi++ simfcgi++.cpp)
target_link_libraries (simfcgi++ ${FCGIXX_LIBRARY} ${FCGI_LIBRARY} ${ZKS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} c++)

add_executable (simfcgi-mt simfcgi-mt.cpp)
target_link_libraries (simfcgi-mt ${FCGI_LIBRARY} ${ZKS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} c++)

