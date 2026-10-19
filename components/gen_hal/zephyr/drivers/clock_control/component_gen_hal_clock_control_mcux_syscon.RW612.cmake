# Add set(CONFIG_USE_component_gen_hal_clock_control_mcux_syscon true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/clock_control_mcux_syscon.c
        )

  

