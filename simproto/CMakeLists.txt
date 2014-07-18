include_directories (${ZKS_INCLUDE_DIR} ${FCGI_INCLUDE_DIR} ${ASIO_INCLUDE_DIR} ${PROTOBUF_INCLUDE_DIR})

SET(_simproto_definitions PACKAGE_NAME=\"asio\" 
        PACKAGE_TARNAME=\"asio\" 
        PACKAGE_VERSION=\"1.10.3\" 
        PACKAGE_STRING=\"asio\ 1.10.3\" 
        PACKAGE_BUGREPORT=\"\" 
        PACKAGE_URL=\"\" 
        PACKAGE=\"asio\" 
        VERSION=\"1.10.3\" 
        _REENTRANT=1 
        ASIO_STANDALONE
        GOOGLE_PROTOBUF_NO_RTTI)



add_executable (simproto simproto.cpp test.pb.cc)
SET_TARGET_PROPERTIES(simproto PROPERTIES
        COMPILE_DEFINITIONS "${_simproto_definitions}")
target_link_libraries (simproto ${FCGI_LIBRARY} ${ZKS_LIBRARY} ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ssl crypto $<$<CXX_COMPILER_ID:Clang>:c++>)

add_executable (proto_test proto_test.cpp test.pb.cc)
SET_TARGET_PROPERTIES(proto_test PROPERTIES
        COMPILE_DEFINITIONS "${_simproto_definitions}")
target_link_libraries (proto_test ${FCGI_LIBRARY} ${ZKS_LIBRARY} ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ssl crypto $<$<CXX_COMPILER_ID:Clang>:c++>)


install (TARGETS simproto proto_test DESTINATION bin)
install (FILES simproto.ini proto_test.ini test.proto DESTINATION etc)
