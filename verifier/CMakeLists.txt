add_executable(verifier_client
        verify_app.c
)

target_include_directories(verifier_client
	PUBLIC SYSTEM ${C_HTTP_INCLUDE_PATHS})

target_link_libraries(verifier_client PUBLIC ${C_HTTP_LINK_LIBRARIES})

