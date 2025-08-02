cmake_minimum_required(VERSION 3.1)

set(PROJECT CashMe_Tests)
enable_language(C CXX)
project(${PROJECT} CXX)

file(GLOB_RECURSE PROJECT_TEST_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_TEST_SRC_RECURSE})

file(GLOB_RECURSE PROJECT_SRC_MODELS 
	${CMAKE_SOURCE_DIR}/src/Models/*.cpp 
	${CMAKE_SOURCE_DIR}/src/Models/*.h 
)
source_group(TREE ${CMAKE_SOURCE_DIR}/src/Models PREFIX CashMe/src/Models FILES ${PROJECT_SRC_MODELS})

file(GLOB_RECURSE PROJECT_SRC_SQLITE3
	${SQLITE3_INCLUDE_DIR}/*.c
	${SQLITE3_INCLUDE_DIR}/*.h
	${SQLITE3_INCLUDE_DIR}/*.hpp
)
source_group(TREE ${SQLITE3_INCLUDE_DIR} PREFIX 3rdparty/sqlite3 FILES ${PROJECT_SRC_SQLITE3})

add_executable(${PROJECT} 
	${PROJECT_TEST_SRC_RECURSE} 
	${PROJECT_SRC_MODELS}
	${PROJECT_SRC_SQLITE3}
)

set_target_properties(${PROJECT} PROPERTIES FOLDER Tests)

#############################################################
#############################################################
#############################################################

target_include_directories(${PROJECT} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src}
	${CMAKE_SOURCE_DIR}
	${EZLIBS_INCLUDE_DIR}
	${SQLITE3_INCLUDE_DIR}
	${SQLITE3_INCLUDE_DIR}/include
)

target_link_libraries(${PROJECT} PRIVATE ${SQLITE3_LIBRARIES})

add_definitions(-DTEST_SAMPLES_PATH="${CMAKE_SOURCE_DIR}/test_samples/")

function(AddTest arg)
  add_test("${arg}" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}" "${arg}")
endfunction(AddTest)

##########################################################
################## DataBase ##############################
##########################################################

AddTest("Test_Database_buildInsertIfNotExistQuery")

