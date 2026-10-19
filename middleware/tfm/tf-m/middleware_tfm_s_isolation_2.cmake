# Add set(CONFIG_USE_middleware_tfm_s_isolation_2 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core/backend_ipc.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core/psa_interface_thread_fn_call.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core/psa_interface_svc.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core/thread.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/lib/runtime/sfn_common_thread.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/lib/runtime/psa_api_ipc.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/include/ffm
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/lib/runtime
        )

  
