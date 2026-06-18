zephyr_include_directories(include)
zephyr_library()

if(CONFIG_SHIELD_DONGLE_SCREEN)

        add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/drivers/display)
        add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/modules/lvgl)

        # Event type and connection observer for battery_bar's connection subscription
        zephyr_library_sources(src/events/split_central_status_changed.c)
        zephyr_library_sources_ifdef(CONFIG_ZMK_SPLIT_ROLE_CENTRAL
                src/split/bluetooth/central_status_changed_observer.c)

endif()