# Add set(CONFIG_USE_component_memfault_sdk_http true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/components/http/src/memfault_http_client.c
          ${CMAKE_CURRENT_LIST_DIR}/components/http/src/memfault_http_client_post_chunk.c
          ${CMAKE_CURRENT_LIST_DIR}/components/http/src/memfault_http_utils.c
          ${CMAKE_CURRENT_LIST_DIR}/components/http/src/memfault_root_certs_der.c
        )

  

