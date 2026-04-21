<!--
Copyright (C) 2024 Nobleo Technology B.V.

SPDX-License-Identifier: Apache-2.0
-->

# nobleo_socketcan_bridge

This package provides functionality to expose CAN frames from SocketCAN to ROS2 topics.

## Overview

This is a from-scratch re-implementation of [socketcan_bridge] from ROS1.
There is a different ROS2 package [ros2_socketcan] which is similar to this package.
The differences between this package and [ros2_socketcan] are:

- No loopback, i.e. CAN frames that are send are not received by the same node
- No lifecycle management, just run it
- Less CPU usage

## Nodes

The main node (`socketcan_bridge`) is also available as dynamically loadable component.
The node `socketcan_bridge_ee` is also provided that uses the `EventsExecutor` that runs a bit more efficient.

### socketcan_bridge

#### Subscribed Topics

* `~/tx` (type depends on `enable_can_fd`)
  When `enable_can_fd=false`: [can_msgs/Frame]
  When `enable_can_fd=true`: [ros2_socketcan_msgs/FdFrame]

#### Published Topics

* `~/rx` (type depends on `enable_can_fd`)
  When `enable_can_fd=false`: [can_msgs/Frame]
  When `enable_can_fd=true`: [ros2_socketcan_msgs/FdFrame]

#### Parameters

* `interface` (default=`can0`)
  Name of the SocketCAN device, by default these devices are named can0 and upwards.
* `enable_can_fd` (default=`false`)
  Enable CAN FD support. When enabled, the socket is configured for CAN FD frames. When disabled, only classic CAN is used. Note: CAN FD and classic CAN are mutually exclusive on the same socket, so this parameter determines the mode for the entire interface.
* `read_timeout` (default=`1.0`)
  Maximum duration in seconds to wait for data on the file descriptor.
* `reconnect_timeout` (default=`5.0`)
  Sleep duration in seconds before reconnecting to the SocketCAN device.

#### Notes on CAN FD Mode

- By default (`enable_can_fd=false`), the bridge operates in classic CAN mode and uses `~/rx` and `~/tx` topics with `can_msgs/Frame` message type (8 bytes max data).
- To enable CAN FD support, launch with `enable_can_fd:=true`. The bridge will use the same `~/rx` and `~/tx` topic names, but with `ros2_socketcan_msgs/FdFrame` message type (up to 64 bytes data).
- Setting `enable_can_fd:=true` requires the SocketCAN interface to support CAN FD. If the interface doesn't support it, a warning will be logged and the bridge will fall back to classic CAN mode.
- Classic and CAN FD cannot be mixed on the same socket, so you must restart the node if you need to switch modes.

[can_msgs/Frame]: https://github.com/ros-industrial/ros_canopen/blob/dashing-devel/can_msgs/msg/Frame.msg
[ros2_socketcan_msgs/FdFrame]: https://github.com/autowarefoundation/ros2_socketcan/blob/main/ros2_socketcan_msgs/msg/FdFrame.msg

[socketcan_bridge]: https://wiki.ros.org/socketcan_bridge

[ros2_socketcan]: https://github.com/autowarefoundation/ros2_socketcan
