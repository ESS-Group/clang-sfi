# We do not copy files over to spare the resources. If all files should be present in the build, activate the following lines and replace the paths accordingly.
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source.cpp" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/run.sh" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/reference.txt" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_test(IntegrationTest_MFC_PP
		${CMAKE_CURRENT_SOURCE_DIR}/run.sh
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}
        )

add_executable(${PROJECT_NAME}_INTEGRATIONTEST_MFC_PP source.cpp)
