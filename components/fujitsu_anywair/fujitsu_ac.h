#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace fujitsu_anywair {

class FujitsuAnywAIRClimate : public climate::Climate, public Component {
 public:
  void setup() override;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void set_uart_parent(uart::UARTComponent *parent) {
    this->set_uart(parent);
  }
  void set_supports_cool(bool supports_cool) { supports_cool_ = supports_cool; }
  void set_supports_heat(bool supports_heat) { supports_heat_ = supports_heat; }

 protected:
  uart::UARTComponent *uart_{nullptr};
  bool supports_cool_{true};
  bool supports_heat_{true};
};

}  // namespace fujitsu_anywair
}  // namespace esphome
