cmake_minimum_required(VERSION 3.10)
project(TractobotsPlannerQt)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED COMPONENTS geometry)

add_executable(tracto_planner_qt
  src/main.cpp
  src/MainWindow.cpp
  src/PathPlanner.cpp
  src/MainWindow.h
  src/PathPlanner.h
  src/Ui_MainWindow.ui
)

qt6_wrap_ui(UISrcs src/Ui_MainWindow.ui)

target_link_libraries(tracto_planner_qt
  Qt6::Widgets
  Boost::geometry
)
