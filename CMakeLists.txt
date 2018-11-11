cmake_minimum_required(VERSION 2.8)
project(sound_engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread")
set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin )
# source files



FILE( GLOB_RECURSE  SRC_FILES
    ${SOURCE_ROOT}/source/*.h
    ${SOURCE_ROOT}/source/*.cpp
)

add_executable(sound_engine ${SRC_FILES})

find_package(ALSA REQUIRED)                                    
if (ALSA_FOUND)                                                
    include_directories(${ALSA_INCLUDE_DIRS})                  
    target_link_libraries (sound_engine ${ALSA_LIBRARIES})      
endif(ALSA_FOUND) 