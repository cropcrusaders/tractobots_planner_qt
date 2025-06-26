add_library(gcode_utils
  src/waypoint_io.cpp
  src/converter.cpp
  src/writer.cpp
  src/advanced_writer.cpp)
target_include_directories(gcode_utils PUBLIC include)

add_executable(gcode_gen src/main.cpp)
target_link_libraries(gcode_gen PRIVATE gcode_utils)

