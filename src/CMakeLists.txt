
add_executable(main main.c)

# use USB for stdio output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# add the SD library
target_link_libraries(main FatFs_SPI)

# add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)

# Specify the output directory for the executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/src)

# add a custom command to copy the executable to the Pi's drive
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -DBIN_PATH=${EXECUTABLE_OUTPUT_PATH}/main.uf2
    -P ${CMAKE_SOURCE_DIR}/copy_uf2_to_pico.cmake
    COMMENT "Copying generated main.uf2 to Pico"
)

# Create a custom target that depends on the custom command
add_custom_target(copy_target ALL DEPENDS main)
