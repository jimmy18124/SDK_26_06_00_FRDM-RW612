# Add set(CONFIG_USE_middleware_tfm_s_loader_service true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service/interface/src/tfm_lds_api.c
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service/auto_generated/intermedia_tfm_loader_service.c
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service/auto_generated/load_info_tfm_loader_service.c
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service/tfm_lds_req_mngr.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service/psa_manifest
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/partitions/loader_service/interface/include
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTFM_PARTITION_LOADER_SERVICE
              )
  
  
  endif()

