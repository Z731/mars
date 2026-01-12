# ros2_ws

This folder is a minimal ROS 2 workspace for building Tencent Mars as part of a ROS 2 package.

## Build

From this directory:

```bash
colcon build --symlink-install
```

## Run

```bash
source install/setup.bash
ros2 run mars_ros2 mars_xlog_node
```

Or via launch:

```bash
source install/setup.bash
ros2 launch mars_ros2 mars_xlog_demo.launch.py
```
