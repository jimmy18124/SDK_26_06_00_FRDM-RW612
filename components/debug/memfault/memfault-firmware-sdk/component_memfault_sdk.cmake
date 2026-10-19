# Add set(CONFIG_USE_component_memfault_sdk true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/arch_arm_cortex_m.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_batched_events.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_build_id.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_compact_log_serializer.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_core_utils.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_custom_data_recording.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_data_export.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_data_packetizer.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_data_source_rle.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_event_storage.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_heap_stats.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_log.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_log_data_source.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_ram_reboot_info_tracking.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_reboot_tracking_serializer.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_sdk_assert.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_self_test.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_self_test_utils.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_serializer_helper.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_task_watchdog.c
          ${CMAKE_CURRENT_LIST_DIR}/components/core/src/memfault_trace_event.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_coredump.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_coredump_regions_armv7.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_coredump_sdk_regions.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_coredump_storage_debug.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_coredump_utils.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_fault_handling_aarch64.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_fault_handling_arm.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_fault_handling_armv7_a_r.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_fault_handling_riscv.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_fault_handling_xtensa.c
          ${CMAKE_CURRENT_LIST_DIR}/components/panics/src/memfault_stdlib_assert.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_base64.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_chunk_transport.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_circular_buffer.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_crc16_ccitt.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_minimal_cbor.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_rle.c
          ${CMAKE_CURRENT_LIST_DIR}/components/util/src/memfault_varint.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/components/include
          ${CMAKE_CURRENT_LIST_DIR}/ports/include
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

  
            if(CONFIG_TOOLCHAIN STREQUAL armgcc)
      target_compile_options(${MCUX_SDK_PROJECT_NAME} PUBLIC
              -g
            )
      endif()
          if(CONFIG_TOOLCHAIN STREQUAL iar)
      target_compile_options(${MCUX_SDK_PROJECT_NAME} PUBLIC
              --diag_suppress=Pe188,Pe1305,Pe128
            )
      endif()
      
  endif()

