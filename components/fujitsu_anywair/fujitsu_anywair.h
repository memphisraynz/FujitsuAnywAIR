#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#include "enums.h"
#include <vector>

namespace esphome {
namespace fujitsu_anywair {

class FujitsuAnywAIRClimate : public climate::Climate, public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop();

  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }
  void set_supports_cool(bool supports_cool) { supports_cool_ = supports_cool; }
  void set_supports_heat(bool supports_heat) { supports_heat_ = supports_heat; }

 protected:
  climate::ClimateMode mode_{climate::CLIMATE_MODE_OFF};
  climate::ClimateFanMode fan_mode_{climate::CLIMATE_FAN_AUTO};
  float current_temperature_{0};

  uart::UARTComponent *uart_{nullptr};
  bool supports_cool_{true};
  bool supports_heat_{true};

  bool validate_message(const uint8_t *buf, size_t len);
  void parse_message(const uint8_t *buf, size_t len);
};

}  // namespace fujitsu_anywair
}  // namespace esphome