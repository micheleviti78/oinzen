cmake_minimum_required(VERSION 3.8)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake)

set(PROJECT_NAME oinzen)

project(${PROJECT_NAME})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
       "Choose the type of build, options are: Debug Release."
       FORCE)
endif()

set(LDSCRIPT ${CMAKE_SOURCE_DIR}/STM32F746NGHx_FLASH.ld)

set(PROJECT_CONFIG_DIR ${CMAKE_SOURCE_DIR}/core/include/conf)

enable_language(C CXX ASM)

set(ARM_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard) 

add_compile_options(${ARM_FLAGS})
add_compile_options(-Wall -Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-fdata-sections -ffunction-sections)

set(LDSPECS nano.specs)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

string(REPLACE ";" " " ARM_LINK_FLAGS "${ARM_FLAGS}")

#set(LINK_FLAGS "${ARM_LINK_FLAGS} -Wl,--gc-sections -Wl,--undefined=uxTopUsedPriority,--undefined=freeRTOSMemoryScheme -Wl,-Map=${PROJECT_NAME}.map,--cref")
set(LINK_FLAGS "${ARM_LINK_FLAGS} -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map,--cref")

set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} -specs=${LDSPECS} -T${LDSCRIPT}")

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/FreeRTOS)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/CMSIS/)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/LwIP/)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/HAL/)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/printf/)

set(FLASH_ADDRESS 0x8000000)

set(STARTUP_SOURCE ${CMAKE_SOURCE_DIR}/boot/startup_stm32f746xx.s)

set(COMMON_SOURCE 
	${CMAKE_SOURCE_DIR}/core/src/app/freertos.c
	${CMAKE_SOURCE_DIR}/core/src/app/lwip.c
	${CMAKE_SOURCE_DIR}/core/src/app/main.c
	${CMAKE_SOURCE_DIR}/core/src/hw/ethernetif.c
	${CMAKE_SOURCE_DIR}/core/src/hw/hal_msp.c
	${CMAKE_SOURCE_DIR}/core/src/hw/hal_timer.c
	${CMAKE_SOURCE_DIR}/core/src/hw/interrupts.c
	${CMAKE_SOURCE_DIR}/core/src/hw/system.c
	${CMAKE_SOURCE_DIR}/core/src/system/syscalls.c
	${CMAKE_SOURCE_DIR}/core/src/util/diag.c
	)
		
add_executable(${PROJECT_NAME} ${COMMON_SOURCE} ${STARTUP_SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC 
${CMAKE_SOURCE_DIR}/core/include/app 
${CMAKE_SOURCE_DIR}/core/include/conf 
${CMAKE_SOURCE_DIR}/core/include/hw 
${CMAKE_SOURCE_DIR}/core/include/util)	

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${LDSCRIPT})

target_link_libraries(${PROJECT_NAME} freertos)
target_link_libraries(${PROJECT_NAME} cmsis)
target_link_libraries(${PROJECT_NAME} lwip) 
target_link_libraries(${PROJECT_NAME} hal)
target_link_libraries(${PROJECT_NAME} printf)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${ARM_SIZE_UTIL} ${PROJECT_NAME}.elf
    COMMAND ${ARM_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    )
    
add_custom_target(flash_ocd)
add_custom_command(
    TARGET flash_ocd
    COMMAND ${OPENOCD} -f board/stm32f746g-disco.cfg -c "program ${PROJECT_NAME}.bin reset exit ${FLASH_ADDRESS}"
 	)

add_custom_target(flash)
add_custom_command(
    TARGET flash
    COMMAND ${STLINK_FLASH} write ${PROJECT_NAME}.bin ${FLASH_ADDRESS}
    )





