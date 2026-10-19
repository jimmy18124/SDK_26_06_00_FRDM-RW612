# Add set(CONFIG_USE_middleware_wireless_framework_platform_mcxw23 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

  if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DENABLE_RAM_VECTOR_TABLE
                        -DgRngIsrPrio_c="((NVIC_DEFAULT_PRIORITY)<<(8U-(uint8_t)__NVIC_PRIO_BITS))"
              )
  
            if(CONFIG_TOOLCHAIN STREQUAL iar)
      target_compile_options(${MCUX_SDK_PROJECT_NAME} PUBLIC
              --diag_suppress=Pa084,Pe188
            )
      endif()
      
  endif()

