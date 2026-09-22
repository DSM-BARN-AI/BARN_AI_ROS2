#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "barn_interfaces/msg/gas_reading.hpp"
#include "barn_interfaces/msg/gas_alert.hpp"

// NH3 임계값 (ppm). 문서 F5.1 기준 예시값.
static constexpr float NH3_THRESHOLD = 25.0;

class GasAlertMonitor : public rclcpp::Node
{
public:
  GasAlertMonitor() : Node("gas_alert_monitor")
  {
    subscription_ = this->create_subscription<barn_interfaces::msg::GasReading>(
      "gas_reading", 10,
      std::bind(&GasAlertMonitor::on_reading, this, std::placeholders::_1));
    alert_publisher_ = this->create_publisher<barn_interfaces::msg::GasAlert>(
      "gas_alert", 10);
    RCLCPP_INFO(this->get_logger(), "gas_alert_monitor 시작 (NH3 임계 %.1f ppm 감시 중)",
      NH3_THRESHOLD);
  }

private:
  void on_reading(const barn_interfaces::msg::GasReading::SharedPtr msg)
  {
    if (msg->nh3_ppm < NH3_THRESHOLD) {
      return;  // 정상 범위면 아무것도 안 함
    }

    auto alert = barn_interfaces::msg::GasAlert();
    alert.header.stamp = this->get_clock()->now();
    alert.level = 2;  // 0=안전, 1=주의, 2=위험
    alert.gas_name = "NH3";
    alert.measured_value = msg->nh3_ppm;
    alert.threshold = NH3_THRESHOLD;
    alert.message = "NH3 농도 임계 초과";

    alert_publisher_->publish(alert);
    RCLCPP_WARN(this->get_logger(), "경보! NH3=%.1f ppm (임계 %.1f ppm 초과)",
      alert.measured_value, alert.threshold);
  }

  rclcpp::Subscription<barn_interfaces::msg::GasReading>::SharedPtr subscription_;
  rclcpp::Publisher<barn_interfaces::msg::GasAlert>::SharedPtr alert_publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<GasAlertMonitor>());
  rclcpp::shutdown();
  return 0;
}
