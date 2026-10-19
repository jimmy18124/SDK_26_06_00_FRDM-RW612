# Add set(CONFIG_USE_middleware_edgefast_bluetooth_crypto_no_psa true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/source/impl/ethermind/crypto/bt_crypto.c
        )

              add_config_file(${CMAKE_CURRENT_LIST_DIR}/../mbedtls3x/port/mcux_mbedtls_config.h "" middleware_edgefast_bluetooth_crypto_no_psa)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/../mbedtls3x/port/mcux_mbedtls_accelerator_config.h "" middleware_edgefast_bluetooth_crypto_no_psa)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/../mbedtls3x/port/mcux_psa_defines.h "" middleware_edgefast_bluetooth_crypto_no_psa)
    
  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/../mbedtls3x/port
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DMBEDTLS_CONFIG_FILE="mcux_mbedtls_config.h"
              )
  
  
  endif()

