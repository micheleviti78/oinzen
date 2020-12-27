set (HAL_SOURCES	
	src/stm32f7xx_hal_adc.c
	src/stm32f7xx_hal_adc_ex.c
	src/stm32f7xx_hal.c
	src/stm32f7xx_hal_cortex.c
	src/stm32f7xx_hal_crc.c
	src/stm32f7xx_hal_crc_ex.c
	src/stm32f7xx_hal_dcmi.c
	src/stm32f7xx_hal_dcmi_ex.c
	src/stm32f7xx_hal_dma2d.c
	src/stm32f7xx_hal_dma.c
	src/stm32f7xx_hal_dma_ex.c
	src/stm32f7xx_hal_dsi.c
	src/stm32f7xx_hal_eth.c
	src/stm32f7xx_hal_exti.c
	src/stm32f7xx_hal_flash.c
	src/stm32f7xx_hal_flash_ex.c
	src/stm32f7xx_hal_gpio.c
	src/stm32f7xx_hal_hcd.c
	src/stm32f7xx_hal_i2c.c
	src/stm32f7xx_hal_i2c_ex.c
	src/stm32f7xx_hal_ltdc.c
	src/stm32f7xx_hal_ltdc_ex.c
	src/stm32f7xx_hal_pwr.c
	src/stm32f7xx_hal_pwr_ex.c
	src/stm32f7xx_hal_qspi.c
	src/stm32f7xx_hal_rcc.c
	src/stm32f7xx_hal_rcc_ex.c
	src/stm32f7xx_hal_rtc.c
	src/stm32f7xx_hal_rtc_ex.c
	src/stm32f7xx_hal_sai.c
	src/stm32f7xx_hal_sai_ex.c
	src/stm32f7xx_hal_sd.c
	src/stm32f7xx_hal_sdram.c
	src/stm32f7xx_hal_spdifrx.c
	src/stm32f7xx_hal_spi.c
	src/stm32f7xx_hal_spi_ex.c
	src/stm32f7xx_hal_tim.c
	src/stm32f7xx_hal_tim_ex.c
	src/stm32f7xx_hal_uart.c
	src/stm32f7xx_hal_uart_ex.c
	src/stm32f7xx_ll_fmc.c
	src/stm32f7xx_ll_sdmmc.c
	src/stm32f7xx_ll_usb.c	
	)
	
add_library(hal STATIC ${HAL_SOURCES})

target_link_libraries(hal cmsis)

target_include_directories(hal
	SYSTEM
	PUBLIC
	${CMAKE_SOURCE_DIR}/lib/hal/include
	${CMAKE_SOURCE_DIR}/lib/hal/include/Legacy
	${CMAKE_SOURCE_DIR}/lib/CMSIS/Device/ST/STM32F7xx/Include
	${CMAKE_SOURCE_DIR}/lib/CMSIS/Include
	${PROJECT_CONFIG_DIR}
	)