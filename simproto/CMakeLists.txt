include_directories (${ZKS_INCLUDE_DIR} ${FCGI_INCLUDE_DIR} ${ASIO_INCLUDE_DIR} ${PROTOBUF_INCLUDE_DIR})

set( LD_LIBS ${FCGI_LIBRARY} ${ZKS_LIBRARY} ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} $<$<CXX_COMPILER_ID:Clang>:c++>)
if(WIN32)
	set( LD_LIBS ${LD_LIBS} wsock32 ws2_32)
endif()

SET(_simproto_definitions PACKAGE_NAME=\"asio\" 
        PACKAGE_TARNAME=\"asio\" 
        PACKAGE_VERSION=\"1.10.3\" 
        PACKAGE_STRING=\"asio\ 1.10.3\" 
        PACKAGE_BUGREPORT=\"\" 
        PACKAGE_URL=\"\" 
        PACKAGE=\"asio\" 
        VERSION=\"1.10.3\"
		_WIN32_WINNT=0x0601
        _REENTRANT=1 
        ASIO_STANDALONE
        GOOGLE_PROTOBUF_NO_RTTI)


add_executable (simproto simproto.cpp test.pb.cc)
SET_TARGET_PROPERTIES(simproto PROPERTIES
        COMPILE_DEFINITIONS "${_simproto_definitions}")
target_link_libraries (simproto ${LD_LIBS})

add_executable (simproto_clnt proto_test.cpp test.pb.cc)
SET_TARGET_PROPERTIES(simproto_clnt PROPERTIES
        COMPILE_DEFINITIONS "${_simproto_definitions}")
target_link_libraries (simproto_clnt ${LD_LIBS})


install (TARGETS simproto simproto_clnt DESTINATION bin)
install (FILES simproto.ini proto_test.ini test.proto DESTINATION etc)
