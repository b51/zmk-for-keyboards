# Build custom ZMK behaviors for this keyboard config.
# target_sources must come after find_package(Zephyr) which defines the 'app' target.
# ZMK's own CMakeLists.txt already called find_package(Zephyr), so 'app' is defined.

if(CONFIG_ZMK_RGB_UNDERGLOW)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rgb_wake.c)
endif()
