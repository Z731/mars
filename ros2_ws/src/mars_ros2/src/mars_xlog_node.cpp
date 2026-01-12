#include <rclcpp/rclcpp.hpp>

#include <filesystem>
#include <string>

#include "mars/comm/xlogger/xlogger.h"
#include "mars/xlog/appender.h"

class MarsXlogNode final : public rclcpp::Node {
 public:
  MarsXlogNode() : rclcpp::Node("mars_xlog_node") {
    log_dir_ = this->declare_parameter<std::string>("log_dir", "./mars_log");
    name_prefix_ = this->declare_parameter<std::string>("name_prefix", "mars_ros2");
    console_log_ = this->declare_parameter<bool>("console_log", true);
    debug_level_ = this->declare_parameter<bool>("debug", false);

    try {
      std::filesystem::create_directories(log_dir_);
    } catch (const std::exception &e) {
      RCLCPP_WARN(this->get_logger(), "Failed to create log_dir '%s': %s", log_dir_.c_str(), e.what());
    }

    // Configure mars xlog.
    if (debug_level_) {
      xlogger_SetLevel(kLevelDebug);
    } else {
      xlogger_SetLevel(kLevelInfo);
    }

    mars::xlog::appender_set_console_log(console_log_);

    mars::xlog::XLogConfig config;
    config.mode_ = mars::xlog::kAppenderAsync;
    config.logdir_ = log_dir_;
    config.nameprefix_ = name_prefix_;
    config.pub_key_ = "";
    config.compress_mode_ = mars::xlog::kZlib;
    config.compress_level_ = 0;
    config.cachedir_ = "";
    config.cache_days_ = 0;

    mars::xlog::appender_open(config);

    timer_ = this->create_wall_timer(std::chrono::seconds(1), [this]() {
      xinfo2("mars xlog from ROS2: tick=%d", tick_++);
    });

    RCLCPP_INFO(this->get_logger(), "mars xlog initialized (log_dir=%s)", log_dir_.c_str());
  }

  ~MarsXlogNode() override {
    mars::xlog::appender_close();
  }

 private:
  std::string log_dir_;
  std::string name_prefix_;
  bool console_log_{true};
  bool debug_level_{false};

  int tick_{0};
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<MarsXlogNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
