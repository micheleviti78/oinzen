set (PRINTF_SOURCES	printf.c)
	
add_library(printf STATIC ${PRINTF_SOURCES})

target_include_directories(printf
	SYSTEM
	PUBLIC
	${CMAKE_SOURCE_DIR}/lib/printf
	)