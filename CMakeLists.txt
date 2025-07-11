cmake_minimum_required(VERSION 3.10)
project(TractobotsPlannerQt)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_GUI "Build Qt GUI" OFF)
enable_testing()
if(BUILD_GUI)
  find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets)
  find_package(Boost REQUIRED COMPONENTS geometry)

  add_executable(tracto_planner_qt
    Scr/main.cpp
    Scr/MainWindow.cpp
    Scr/PathPlanner.cpp
    Scr/MainWindow.h
    Scr/PathPlanner.h
    Scr/Ui_MainWindow.ui
  )

  qt6_wrap_ui(UISrcs Scr/Ui_MainWindow.ui)

  target_link_libraries(tracto_planner_qt
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Boost::geometry
  )
endif()

add_subdirectory(src/gcode_gen)

add_executable(unit_tests
  tests/test_absolute.cpp
  tests/test_subprog.cpp
  tests/main.cpp
)
target_link_libraries(unit_tests PRIVATE gcode_utils)
add_test(NAME unit_tests COMMAND unit_tests)
