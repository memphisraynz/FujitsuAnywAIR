#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace fujitsu_anywair {

class FujitsuAnywAIRClimate : public climate::Climate, public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }
  void set_supports_cool(bool supports_cool) { supports_cool_ = supports_cool; }
  void set_supports_heat(bool supports_heat) { supports_heat_ = supports_heat; }

 protected:
  uart::UARTComponent *uart_{nullptr};
  bool supports_cool_{true};
  bool supports_heat_{true};

  // Communication helpers
  void write_bytes(const uint8_t* data, size_t length);
  int read_bytes(uint8_t* buffer, size_t length, int timeout_ms);

  bool send_command(const std::vector<uint8_t> &command);
  bool read_response(std::vector<uint8_t> &response, int timeout_ms = 1000);
};

}  // namespace fujitsu_anywair
}  // namespace esphome
