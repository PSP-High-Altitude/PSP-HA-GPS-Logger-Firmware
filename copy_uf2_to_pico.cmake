set(PICO_MOUNT_PATH "D:")
set(INFO_UF2_PATH "${PICO_MOUNT_PATH}/INFO_UF2.TXT")

if (EXISTS ${INFO_UF2_PATH})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy 
        ${BIN_PATH} ${PICO_MOUNT_PATH}
        RESULT_VARIABLE result
    )
    if (result EQUAL 0)
        message("Copied ${BIN_PATH} to ${PICO_MOUNT_PATH}")
    else()
        message("Failed to copy ${BIN_PATH} to ${PICO_MOUNT_PATH}")
    endif()
else()
    message("${INFO_UF2_PATH} not found. Skipping copy.")
endif()
