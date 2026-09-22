#include <chrono>
#include <memory>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "barn_interfaces/msg/gas_reading.hpp"

using namespace std::chrono_literals;

class GasPublisher : public rclcpp::Node
{
public:
  GasPublisher() : Node("gas_publisher"), gen_(rd_())
  {
    publisher_ = this->create_publisher<barn_interfaces::msg::GasReading>(
      "gas_reading", 10);
    timer_ = this->create_wall_timer(
      1s, std::bind(&GasPublisher::publish_reading, this));
    RCLCPP_INFO(this->get_logger(), "gas_publisher 시작 (더미 데이터 발행 중)");
  }

private:
  void publish_reading()
  {
    std::uniform_real_distribution<float> nh3(5.0, 30.0);
    std::uniform_real_distribution<float> co2(400.0, 3000.0);

    auto msg = barn_interfaces::msg::GasReading();
    msg.header.stamp = this->get_clock()->now();
    msg.header.frame_id = "sensor_link";
    msg.nh3_ppm = nh3(gen_);
    msg.co2_ppm = co2(gen_);

    publisher_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "발행: NH3=%.1f ppm, CO2=%.1f ppm",
      msg.nh3_ppm, msg.co2_ppm);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<barn_interfaces::msg::GasReading>::SharedPtr publisher_;
  std::random_device rd_;
  std::mt19937 gen_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<GasPublisher>());
  rclcpp::shutdown();
  return 0;
}
