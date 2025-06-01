add_library(gcode_utils
  src/waypoint_io.cpp
  src/converter.cpp)
target_include_directories(gcode_utils PUBLIC include)

add_executable(gcode_gen src/main.cpp)
target_link_libraries(gcode_gen PRIVATE gcode_utils)

enable_testing()
add_executable(gcode_gen_tests tests/test_converter.cpp)
target_link_libraries(gcode_gen_tests PRIVATE gcode_utils)
add_test(NAME gcode_gen_tests COMMAND gcode_gen_tests)
