add_executable(verifier_client
	app.c
)

target_include_directories(verifier_client
	PUBLIC SYSTEM ${C_EG_INCLUDE_PATHS})

target_link_libraries(verifier_client PUBLIC ${C_EG_LINK_LIBRARIES})

