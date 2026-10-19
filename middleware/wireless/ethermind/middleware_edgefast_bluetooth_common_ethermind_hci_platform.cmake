# Add set(CONFIG_USE_middleware_edgefast_bluetooth_common_ethermind_hci_platform true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/port/pal/mcux/bluetooth/hci_platform.c
        )

  

