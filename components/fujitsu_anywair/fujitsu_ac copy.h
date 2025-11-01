#pragma once

namespace esphome {
namespace fujitsu_ac {

class FujitsuACClimate : public climate::Climate, public Component {
 public:
  void setup() override;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }

 protected:
  uart::UARTComponent *uart_;
};

}  // namespace fujitsu_ac
}  // namespace esphome
