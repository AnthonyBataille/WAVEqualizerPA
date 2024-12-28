cmake_minimum_required(VERSION 3.15)

project(WAVEqualizerPA VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES "main.cpp" "devices.cpp" "stream.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

set(PORTAUDIO_ROOT "${PROJECT_SOURCE_DIR}/portaudio")
set(PORTAUDIO_DLL "${PORTAUDIO_ROOT}/build/msvc/x64/Debug/portaudio_x64.dll")

add_custom_command(TARGET WAVEqualizerPA POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                           ${PORTAUDIO_DLL}
                           $<TARGET_FILE_DIR:WAVEqualizerPA>)

include_directories(${PORTAUDIO_ROOT}/include)

message(STATUS "${PROJECT_SOURCE_DIR}")

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D__SCL_SECURE_NO_WARNINGS -DNO_WARN_MBCS_MFC_DEPRECATION)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX")

    target_link_libraries(${PROJECT_NAME} ${PORTAUDIO_ROOT}/build/msvc/x64/Debug/portaudio_x64.lib)
else()
	message(FATAL "MSVC undefined")
endif()
