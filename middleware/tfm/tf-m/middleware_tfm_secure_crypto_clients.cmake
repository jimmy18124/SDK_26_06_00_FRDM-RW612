# Add set(CONFIG_USE_middleware_tfm_secure_crypto_clients true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/interface/src/tfm_crypto_api.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/include
          ${CMAKE_CURRENT_LIST_DIR}/config
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/include
          ${CMAKE_CURRENT_LIST_DIR}/lib/fih/inc
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/lib/runtime/include
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/include
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common/Device/Config
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common/Device/Include
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/include/interface
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/driver
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto
          ${CMAKE_CURRENT_LIST_DIR}/interface/src
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/platform
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE
                        -DPLATFORM_DEFAULT_CRYPTO_KEYS
                        -DMBEDTLS_PSA_CRYPTO_DRIVERS
              )
  
  
  endif()

