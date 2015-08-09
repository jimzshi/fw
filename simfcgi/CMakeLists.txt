include_directories (${ZKS_INCLUDE_DIR} ${FASTCGI_INCLUDE_DIR})

add_executable (simfcgi-mt simfcgi-mt.cpp)

set( LD_LIBS ${LD_LIBS} ${FASTCGI_LIBRARY} ${ZKS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
	set( LD_LIBS ${LD_LIBS} wsock32 ws2_32)
endif()

target_link_libraries (simfcgi-mt ${LD_LIBS} $<$<CXX_COMPILER_ID:Clang>:c++>)

install (TARGETS simfcgi-mt DESTINATION bin)
install (FILES simfcgi.ini DESTINATION etc)
