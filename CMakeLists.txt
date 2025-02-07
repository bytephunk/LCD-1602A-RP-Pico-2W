cmake_minimum_required(VERSION 3.13)

#enable debug
set(CMAKE_BUILD_TYPE "Debug")

#change this according to your board
set( PICO_BOARD "pico2_w" )

include(pico_sdk_import.cmake)

project(LCD1602A)
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(LCD1602A lcd_example.c  LCD1602A.c)
pico_generate_pio_header(LCD1602A ${CMAKE_CURRENT_LIST_DIR}/LCD1602A.pio)

# pull in common dependencies
target_link_libraries(LCD1602A pico_stdlib hardware_pio )


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(LCD1602A)

target_compile_definitions(LCD1602A PRIVATE
        PICO_ENTER_USB_BOOT_ON_EXIT=1
)
