include_directories ( include )
include_directories ( ${CMAKE_BINARY_DIR}/client )

FLATBUFFERS_GENERATE_C_HEADERS ( OXIDE_CLIENT_IDL idl/test.idl )
add_custom_target(oxide_client_flatbuffers DEPENDS ${OXIDE_CLIENT_IDL_OUTPUTS})

add_executable ( oxide_client src/main.cpp )
add_dependencies( oxide_client oxide_client_flatbuffers )
target_link_libraries ( oxide_client  )