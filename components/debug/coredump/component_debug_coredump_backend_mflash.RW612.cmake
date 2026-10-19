# Add set(CONFIG_USE_component_debug_coredump_backend_mflash true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/coredump_core.c
          ${CMAKE_CURRENT_LIST_DIR}/coredump_memory_regions.c
          ${CMAKE_CURRENT_LIST_DIR}/coredump_backend_mflash.c
          ${CMAKE_CURRENT_LIST_DIR}/coredump.c
          ${CMAKE_CURRENT_LIST_DIR}/error_stack_frame.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

  
