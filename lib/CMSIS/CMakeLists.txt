set (CMSIS_SOURCES CMSIS_RTOS/cmsis_os.c
)
	
add_library(cmsis STATIC ${CMSIS_SOURCES})

target_link_libraries(cmsis freertos)

target_include_directories(cmsis
	SYSTEM
	PUBLIC
	${CMAKE_SOURCE_DIR}/lib/CMSIS/Device/ST/STM32F7xx/Include/
	${CMAKE_SOURCE_DIR}/lib/CMSIS/Include/
	${CMAKE_SOURCE_DIR}/lib/CMSIS/CMSIS_RTOS/
	)