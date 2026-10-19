# Add set(CONFIG_USE_middleware_tfm_s_partition_load_ps true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/protected_storage/auto_generated/load_info_tfm_protected_storage.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/protected_storage/psa_manifest
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/protected_storage/crypto
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/protected_storage
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/protected_storage/auto_generated
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DPS_ENCRYPTION
                        -DPS_ROLLBACK_PROTECTION
                        -DTFM_PARTITION_PROTECTED_STORAGE
              )
  
  
  endif()

