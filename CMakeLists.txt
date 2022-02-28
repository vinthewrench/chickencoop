#

# export CC=/usr/local/clang_9.0.0/bin/clang
# export CXX=/usr/local/clang_9.0.0/bin/clang++
#SET (CMAKE_C_COMPILER             "/usr/lib/llvm/10/bin/clang")
#SET (CMAKE_CXX_COMPILER           "/usr/lib/llvm/10/bin/clang++")


option(CROSS_COMPILE "Cross Compile from Mac" OFF) #OFF by default
if(CROSS_COMPILE)
    #add some compilation flags
    
SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_CXX_COMPILER            "clang++")

include_directories("/Users/vinnie/Desktop/RasberryPi/sysroot/usr/include")
link_directories("/Users/vinnie/Desktop/RasberryPi/sysroot/usr/lib")
#SET(CMAKE_SYSROOT /Users/vinnie/Desktop/RasberryPi/sysroot/)
set(CMAKE_SYSTEM_PROCESSOR arm)

else()
    #add some other compilation flags
SET (CMAKE_C_COMPILER             "/usr/local/clang_12.0.1/bin/clang")
SET (CMAKE_CXX_COMPILER            "/usr/local/clang_12.0.1/bin/clang++")

include_directories(/usr/include)
link_directories(/usr/lib)

endif(CROSS_COMPILE)
unset(CROSS_COMPILE CACHE) # <---- this is the important!!

cmake_minimum_required(VERSION 3.0)

project(coopmgr)

set(SOURCES
	coopserver/src/main.cpp
	coopserver/src/ServerCommandsLineFunctions.cpp
	coopserver/src/ServerNouns.cpp
	coopserver/CoopMgr/CoopMgr.cpp
	coopserver/CoopMgr/CoopMgrDB.cpp
	coopserver/CoopMgr/ErrorMgr.cpp
	coopserver/CoopMgr/CoopMgrAPISecretMgr.cpp
	coopserver/CoopMgr/sunset.cpp
	coopserver/CoopMgr/SolarTimeMgr.cpp
	coopserver/CoopMgr/CoopMgrDB.cpp
	coopserver/CoopMgr/CoopDevices.cpp
	coopserver/CoopMgr/Action.cpp
	coopserver/CoopMgr/Event.cpp
	coopserver/CoopMgr/DoorMgr.cpp
	coopserver/CoopMgr/I2C.cpp
	coopserver/CoopMgr/TMP102.cpp
	coopserver/CoopMgr/CPUInfo.cpp
	coopserver/CoopMgr/TempSensor.cpp
#	coopserver/CoopMgr/WittyPi3.cpp
	coopserver/CoopMgr/PiJuice.cpp
 	coopserver/CoopMgr/QwiicButton.cpp
#	coopserver/CoopMgr/RPi_RelayHat.cpp
	coopserver/CoopMgr/RelayHat.cpp
	coopserver/CoopMgr/GPIO.cpp
	Submodule/FooServer/src/RESTutils.cpp
	Submodule/FooServer/src/ServerCmdQueue.cpp
	Submodule/FooServer/src/TCPClientInfo.cpp
	Submodule/FooServer/src/TCPServer.cpp
	Submodule/FooServer/src/REST/http_parser.c
	Submodule/FooServer/src/REST/REST_URL.cpp
	Submodule/FooServer/src/REST/RESTServerConnection.cpp
 	Submodule/FooServer/src/REST/yuarel.c
 	Submodule/FooServer/src/Telnet/CmdLineBuffer.cpp
 	Submodule/FooServer/src/Telnet/CmdLineBuffer.cpp
 	Submodule/FooServer/src/Telnet/CmdLineHelp.cpp
 	Submodule/FooServer/src/Telnet/CmdLineMgr.cpp
 	Submodule/FooServer/src/Telnet/CmdLineRegistry.cpp
 	Submodule/FooServer/src/Telnet/TelnetServerConnection.cpp
 	Submodule/FooServer/src/Telnet/libtelnet.c
 	Submodule/FooServer/src/hash-library/sha256.cpp
	Submodule/FooServer/src/TimeStamp.cpp
	Submodule/FooServer/src/LogMgr.cpp
 	)

add_executable(coopmgr ${SOURCES})


set_target_properties(coopmgr PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )
 
set_target_properties(coopmgr PROPERTIES
			OUTPUT_NAME "coopmgr"
			)

           
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(coopmgr PRIVATE sqlite3 Threads::Threads gpiod)

target_include_directories(coopmgr
	PRIVATE 
	coopserver/src
	coopserver/CoopMgr
	Submodule/FooServer/src
	Submodule/FooServer/src/hash-library 
	Submodule/FooServer/src/REST
	Submodule/FooServer/src/Telnet

	)

set(CMAKE_BINARY_DIR "bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Assets ${CMAKE_BINARY_DIR}
)

add_dependencies(coopmgr copy_assets)
