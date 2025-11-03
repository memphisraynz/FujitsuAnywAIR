#pragma once

#include <set>
#include <string>
#include <vector>

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace fujitsu_anywair {

using climate::ClimateCall;
using climate::ClimateMode;
using climate::ClimatePreset;
using climate::ClimateSwingMode;
using climate::ClimateFanMode;
using climate::ClimateTraits;

// Forward declaration for sensors if needed
class Sensor;

class FujitsuAnywAIRClimate : public climate::Climate, public uart::UARTDevice, public Component {
 public:
  void dump_config() override;
  void setup() override;
  void loop() override;
  void control(const ClimateCall &call) override;
  ClimateTraits traits() override;

  void set_supported_modes(const std::vector<ClimateMode> &modes);
  void set_supported_swing_modes(const std::vector<ClimateSwingMode> &modes);
  void set_supported_presets(const std::vector<ClimatePreset> &presets);
  void set_custom_fan_modes(const std::vector<std::string> &modes);

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }

  protected:
  uart::UARTComponent *uart_{nullptr};

  std::set<ClimateMode> supported_modes_;
  std::set<ClimatePreset> supported_presets_;
  std::set<ClimateSwingMode> supported_swing_modes_;
  std::vector<std::string> supported_custom_fan_modes_{};

  // Current parsed states
  ClimateMode mode_{ClimateMode::CLIMATE_MODE_OFF};
  ClimateFanMode fan_mode_{climate::CLIMATE_FAN_AUTO};
  float current_temperature_{0.0f};

  // Parsing and communication helpers
  bool validate_message(const uint8_t *buf, size_t len);
  void parse_message(const uint8_t *buf, size_t len);

  void write_bytes(const uint8_t* data, size_t length);
  int read_bytes(uint8_t* buffer, size_t length, int timeout_ms);
  bool send_command(const std::vector<uint8_t> &command);
  bool read_response(std::vector<uint8_t> &response, int timeout_ms);
};

}  // namespace fujitsu_anywair
}  // namespace esphome