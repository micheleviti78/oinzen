set (FREERTOS_SOURCES	
	croutine.c
	event_groups.c
	list.c
	queue.c
	stream_buffer.c
	tasks.c
	timers.c
	portable/GCC/ARM_CM7/r0p1/port.c
	)

set (FREERTOS_MEMMANG_SOURCE portable/MemMang/heap_useNewlib_ST.c)
	
add_library(freertos STATIC ${FREERTOS_SOURCES} ${FREERTOS_MEMMANG_SOURCE})

target_include_directories(freertos
	SYSTEM
	PUBLIC
	${CMAKE_SOURCE_DIR}/lib/FreeRTOS/portable/GCC/ARM_CM7/r0p1
	${CMAKE_SOURCE_DIR}/lib/FreeRTOS/include
	${PROJECT_CONFIG_DIR}
	)