# Add set(CONFIG_USE_component_gen_hal_flash_flexspi_nor true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/flash_mcux_flexspi_nor.c
          ${CMAKE_CURRENT_LIST_DIR}/flash_page_layout.c
          ${CMAKE_CURRENT_LIST_DIR}/flash_util.c
          ${CMAKE_CURRENT_LIST_DIR}/jesd216.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

  
