# Add set(CONFIG_USE_middleware_tfm_s_crypto true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/auto_generated/intermedia_tfm_crypto.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/auto_generated/load_info_tfm_crypto.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/psa_driver_api/tfm_builtin_key_loader.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_aead.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_alloc.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_asymmetric.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_cipher.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_hash.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_init.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_key_derivation.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_key_management.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_library.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_mac.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/crypto_rng.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/interface/include/psa
          ${CMAKE_CURRENT_LIST_DIR}/interface/include
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/include
          ${CMAKE_CURRENT_LIST_DIR}/config
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/include
          ${CMAKE_CURRENT_LIST_DIR}/lib/fih/inc
          ${CMAKE_CURRENT_LIST_DIR}/platform/include
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/lib/runtime/include
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/crypto/psa_driver_api
          ${CMAKE_CURRENT_LIST_DIR}/lib/ext/mbedcrypto/mbedcrypto_config
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common/Device/Config
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common/Device/Include
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/common
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/include/interface
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTFM_PARTITION_CRYPTO
                        -DPLATFORM_DEFAULT_CRYPTO_KEYS
                        -DMBEDTLS_PSA_CRYPTO_DRIVERS
                        -DPSA_CRYPTO_DRIVER_TFM_BUILTIN_KEY
                        -DPSA_CRYPTO_DRIVER_TFM_BUILTIN_KEY_LOADER
                        -DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_INFO
                        -DPS_CRYPTO_AEAD_ALG=PSA_ALG_CCM
              )
  
  
  endif()

