cmake_minimum_required(VERSION 3.1)

set(PROJECT_TEST LCLBroker_Tests)
enable_language(C CXX)
project(${PROJECT} CXX)

file(GLOB_RECURSE PROJECT_TEST_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_TEST_SRC_RECURSE})

file(GLOB PROJECT_TEST_UTILS 
	${CMAKE_CURRENT_SOURCE_DIR}/../src/Utils/Utils.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/../src/Utils/Utils.h 
)
source_group(LCLBroker/Utils FILES ${PROJECT_TEST_UTILS})

add_executable(${PROJECT_TEST} ${PROJECT_TEST_SRC_RECURSE} ${PROJECT_TEST_UTILS})

set_target_properties(${PROJECT_TEST} PROPERTIES FOLDER Tests)

#############################################################
#############################################################
#############################################################

target_include_directories(${PROJECT_TEST} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}
	${CTOOLS_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
)

target_link_libraries(${PROJECT_TEST}
	${CTOOLS_LIBRARIES}
)

## copy app dll in the CTEST dir
add_custom_command( 
	TARGET ${PROJECT_TEST} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_RUNTIME_DLLS:${PROJECT}>
		$<TARGET_FILE_DIR:${PROJECT_TEST}>
	COMMAND_EXPAND_LISTS)

add_definitions(-DTEST_SAMPLES_PATH="${CMAKE_SOURCE_DIR}/test_samples/")

function(SetTest arg)
	add_test("${arg}" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TEST}" "${arg}")
endfunction()

##########################################################
## parseDescription ######################################
##########################################################

## test_parseDescription
SetTest("test_parseDescription_0")
SetTest("test_parseDescription_1")
