cmake_minimum_required(VERSION 3.1)

project(xiaomaoMsg)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
else()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set(XIAOMAOMSGAPP_DIR ${CMAKE_CURRENT_LIST_DIR}/src/xiaomaoMsgApp)
set(XIAOMAOMSGLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/src/xiaomaoMsgLib)

file(GLOB XIAOMAOMSGLIB_SRC 
	${XIAOMAOMSGLIB_DIR}/*.cpp
	${XIAOMAOMSGLIB_DIR}/include/*.h
	)

file(GLOB SERVER_SRC 
	${CMAKE_CURRENT_LIST_DIR}/src/testServer/*.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/testServer/*.h
	)

file(GLOB XIAOMAOMSGAPP_SRC 
	${XIAOMAOMSGAPP_DIR}/*.cpp
	${XIAOMAOMSGAPP_DIR}/include/*.h
	)
	
add_library(xiaomaoMsgLib STATIC ${XIAOMAOMSGLIB_SRC} )

set(NNG_STATIC_LIB ON)


if(LINUX)
	set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/mbedtls/linux)
else()
	set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/mbedtls/win64)
endif()

add_subdirectory(3rdparty/nng)

set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT xiaomaoMsgApp)
set(VS_STARTUP_PROJECT xiaomaoMsgApp)

INCLUDE_DIRECTORIES(${XIAOMAOMSGLIB_DIR}/include)
INCLUDE_DIRECTORIES(${XIAOMAOMSGAPP_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src/timeUtils/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/3rdparty/nng/nnglib/include)

add_executable(testServer ${SERVER_SRC} )
add_executable(xiaomaoMsgApp ${XIAOMAOMSGAPP_SRC} )

add_dependencies(xiaomaoMsgApp xiaomaoMsgLib)
add_dependencies(testServer xiaomaoMsgLib)

target_link_libraries(xiaomaoMsgLib nng)
target_link_libraries(xiaomaoMsgApp xiaomaoMsgLib)
target_link_libraries(testServer xiaomaoMsgLib)

