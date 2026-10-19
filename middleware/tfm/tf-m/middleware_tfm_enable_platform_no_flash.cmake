# Add set(CONFIG_USE_middleware_tfm_enable_platform_no_flash true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

  if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DPLATFORM_NO_FLASH
                        -DPS_RAM_FS
                        -DITS_RAM_FS
                        -DOTP_NV_COUNTERS_RAM_EMULATION
              )
  
  
  endif()

