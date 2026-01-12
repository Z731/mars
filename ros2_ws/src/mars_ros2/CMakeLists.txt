cmake_minimum_required(VERSION 3.8)
project(mars_ros2)

# This package builds the upstream Mars CMake project as a subproject.
# NOTE: Mars CMakeLists were patched to not override CMAKE_INSTALL_PREFIX when consumed.

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Repo layout:
#   <repo_root>/mars            (upstream Mars CMake project)
#   <repo_root>/ros2_ws/src/... (this ROS2 workspace)
get_filename_component(MARS_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(MARS_SOURCE_DIR "${MARS_REPO_ROOT}/mars")

if(NOT EXISTS "${MARS_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "Mars source not found at ${MARS_SOURCE_DIR}. Expected ${MARS_SOURCE_DIR}/CMakeLists.txt")
endif()

# Build Mars as a subproject.
# The binary dir name is kept unique to avoid collisions with other packages.
add_subdirectory("${MARS_SOURCE_DIR}" "mars_build")

add_executable(mars_xlog_node src/mars_xlog_node.cpp)

target_include_directories(mars_xlog_node PRIVATE
  "${MARS_REPO_ROOT}"          # for includes like <mars/comm/...>
  "${MARS_SOURCE_DIR}"
)

# Ensure XLOGGER_TAG is always defined for translation units using xlogger.
target_compile_definitions(mars_xlog_node PRIVATE XLOGGER_TAG="\"mars_ros2\"")

# Mars produces static libraries via its own CMake targets.
# Commonly needed libs: comm, xlog, mars-boost, libzstd_static.
target_link_libraries(mars_xlog_node PRIVATE
  xlog
  comm
  mars-boost
  libzstd_static
  ZLIB::ZLIB
  Threads::Threads
)

ament_target_dependencies(mars_xlog_node rclcpp)

install(TARGETS mars_xlog_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
