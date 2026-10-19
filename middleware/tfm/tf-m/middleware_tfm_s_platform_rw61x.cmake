# Add set(CONFIG_USE_middleware_tfm_s_platform_rw61x true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/crypto_keys.c
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/nv_counters.c
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common/CMSIS_Driver/Driver_USART.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/include
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/platform/include
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DUSE_ELS_PKC_HUK
                        -DUSE_ELS_PKC_IAK
                        -DHARDENING_MACROS_ENABLED
                        -DPSA_WANT_ECC_SECP_R1_224
              )
  
  
  endif()

