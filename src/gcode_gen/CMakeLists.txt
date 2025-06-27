add_library(gcode_utils
  src/waypoint_io.cpp
  src/converter.cpp
  src/gps_transforms.cpp)
target_include_directories(gcode_utils PUBLIC include)

add_executable(gcode_gen src/main.cpp)
target_link_libraries(gcode_gen PRIVATE gcode_utils)

enable_testing()
add_executable(test_converter tests/test_converter.cpp)
target_link_libraries(test_converter PRIVATE gcode_utils)
add_test(NAME test_converter COMMAND test_converter)

add_executable(test_gps_transforms tests/test_gps_transforms.cpp)
target_link_libraries(test_gps_transforms PRIVATE gcode_utils)
add_test(NAME test_gps_transforms COMMAND test_gps_transforms)
