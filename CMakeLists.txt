cmake_minimum_required(VERSION 3.15)
project(ByteBoi-Library)
set(PORT /dev/ttyS7)
set(DEVICE cm:esp32:wheelson)
set(CMAKE_CXX_STANDARD 11)

if(WIN32 OR CYGWIN)
    set(Arduino15 $ENV{APPDATA}/../Local/Arduino15)
    set(ArduinoLibs $ENV{USERPROFILE}/Documents/Arduino/libraries)

    string(REPLACE "\\" "/" Arduino15 ${Arduino15})
    string(REPLACE "\\" "/" ArduinoLibs ${ArduinoLibs})

    if(${CYGWIN})
        string(REGEX REPLACE "^([A-Za-z]):" "/cygdrive/\\1" Arduino15 ${Arduino15})
        string(REGEX REPLACE "^([A-Za-z]):" "/cygdrive/\\1" ArduinoLibs ${ArduinoLibs})
    endif()
else()
    set(Arduino15 ~/.arduino15)
    set(ArduinoLibs ~/Arduino/libraries)
endif()

include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/Wire/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/FS/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/SD/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/SPI/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/WiFi/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/SPIFFS/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/HTTPClient/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/WebServer/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/DNSServer/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/CircuitOS/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/TFT_eSPI)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/TFT_eSPI/Extensions)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/LovyanGFX/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/U8g2_for_TFT_eSPI/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/FDK-AAC/src)

include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/cores/esp32)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include/esp32)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include/freertos)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include/driver)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include/driver/driver)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include/spi_flash)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/tools/sdk/include/esp32-camera/)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/SPIFFS/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/Update/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/cpp-properties/src)
include_directories(${Arduino15}/packages/cm/hardware/esp32/1.7.5/libraries/CircuitMess-Audio/src)


file(GLOB_RECURSE SOURCES "src/**.cpp" "src/**.c" "src/**.hpp" "src/**.h")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

add_definitions(-DDEBUG)
add_definitions(-DESP32)
add_definitions(-DCIRCUITOS_LOVYANGFX)
add_definitions(-DCIRCUITOS_PIEZO_DAC)
add_definitions(-DESP_PLATFORM)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_PROJECT_NAME}.ino PROPERTIES LANGUAGE CXX)

add_custom_command(OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.ino.elf
        COMMAND arduino-cli compile --fqbn ${DEVICE} --output-dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_PROJECT_NAME}.ino
        DEPENDS ${CMAKE_PROJECT_NAME}.ino ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target(CMBuild
        SOURCES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.ino.elf)

add_custom_target(CMUpload ALL
        COMMAND arduino-cli upload --fqbn ${DEVICE} -p ${PORT} --input-dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        DEPENDS CMBuild
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(${CMAKE_PROJECT_NAME}.bin ${CMAKE_PROJECT_NAME}.ino ${SOURCES})