// Copyright (C) 2024 Nobleo Technology B.V.
//
// SPDX-License-Identifier: Apache-2.0

#include "nobleo_socketcan_bridge/socketcan_bridge_node.hpp"

#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp_components/register_node_macro.hpp>

namespace nobleo_socketcan_bridge
{

SocketCanBridgeNode::SocketCanBridgeNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("socketcan_bridge", options),
  updater_(this),
  enable_can_fd_(this->declare_parameter("enable_can_fd", false)),
  can_pub(enable_can_fd_ ? nullptr : this->create_publisher<can_msgs::msg::Frame>("~/rx", 100)),
  can_fd_pub(
    enable_can_fd_ ? this->create_publisher<ros2_socketcan_msgs::msg::FdFrame>("~/rx", 100)
                   : nullptr),
  bridge(
    this->get_logger(), this->get_clock(), this->declare_parameter("interface", "can0"),
    this->declare_parameter("read_timeout", 1.0), this->declare_parameter("reconnect_timeout", 5.0),
    enable_can_fd_, [this](const can_msgs::msg::Frame & msg) {
      if (can_pub) {
        can_pub->publish(msg);
      }
    },
    [this](const ros2_socketcan_msgs::msg::FdFrame & msg) {
      if (can_fd_pub) {
        can_fd_pub->publish(msg);
      }
    }),
  can_sub(enable_can_fd_ ? nullptr : this->create_subscription<can_msgs::msg::Frame>(
                                      "~/tx", 100,
                                      [this](can_msgs::msg::Frame::ConstSharedPtr msg) {
                                        bridge.send(*msg);
                                      })),
  can_fd_sub(enable_can_fd_ ? this->create_subscription<ros2_socketcan_msgs::msg::FdFrame>(
                                "~/tx", 100,
                                [this](ros2_socketcan_msgs::msg::FdFrame::ConstSharedPtr msg) {
                                  bridge.send(*msg);
                                })
                            : nullptr)
{
  RCLCPP_INFO(
    this->get_logger(), "SocketCAN bridge mode: %s (topics: ~/tx and ~/rx)",
    enable_can_fd_ ? "CAN FD" : "Classic CAN");

  updater_.setHardwareID("SocketCan");
  updater_.add("SocketCan", [this](auto & stat) { this->produceDiagnostics(stat); });
}

void SocketCanBridgeNode::produceDiagnostics(diagnostic_updater::DiagnosticStatusWrapper & status)
{
  auto can_state = bridge.getState();
  switch (can_state.state) {
    case CanState::OKAY:
      status.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "CAN interface is up");
      break;
    case CanState::WARN:
      status.summary(
        diagnostic_msgs::msg::DiagnosticStatus::WARN, "CAN interface is in warning state");
      break;
    case CanState::ERROR:
      status.summary(
        diagnostic_msgs::msg::DiagnosticStatus::ERROR, "CAN interface is in error state");
      break;
    case CanState::CONNECTION_ERROR:
      status.summary(
        diagnostic_msgs::msg::DiagnosticStatus::ERROR, "Error connecting to CAN interface");
      break;
  }

  status.add("Error class", can_state.error_class);
  status.add("Controller error", can_state.controller_error);
  status.add("Protocol error", can_state.protocol_error);
  status.add("TX error counter", can_state.tx_error_counter);
  status.add("RX error counter", can_state.rx_error_counter);
}

}  // namespace nobleo_socketcan_bridge
RCLCPP_COMPONENTS_REGISTER_NODE(nobleo_socketcan_bridge::SocketCanBridgeNode)
