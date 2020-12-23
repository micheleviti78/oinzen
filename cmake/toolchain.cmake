set(CMAKE_SYSTEM_NAME Generic) # 'Generic' is used for embedded systems
set(CMAKE_SYSTEM_PROCESSOR arm)

# tells CMake not to try to link executables during its interal checks
# things are not going to link properly without a linker script
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOOLCHAIN_PATH /usr)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-g++)
set(ARM_SIZE_UTIL ${TOOLCHAIN_PATH}/bin/arm-none-eabi-size)
set(ARM_OBJCOPY ${TOOLCHAIN_PATH}/bin/arm-none-eabi-objcopy)
set(ARM_OBJDUMP ${TOOLCHAIN_PATH}/bin/arm-none-eabi-objdump)

set(OPENOCD_PATH /usr/local/bin)
set(OPENOCD ${OPENOCD_PATH}/openocd)

set(STLINK_PATH /usr/local/bin)
set(STLINK_FLASH ${STLINK_PATH}/st-flash)