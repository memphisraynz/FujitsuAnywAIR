#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace fujitsu_anywair {

using climate::ClimateCall;
using climate::ClimateMode;
using climate::ClimateModeMask;
using climate::ClimatePreset;
using climate::ClimatePresetMask;
using climate::ClimateSwingModeMask;
using climate::ClimateTraits;

// Forward declaration for sensor class if needed
class Sensor;

class FujitsuAnywAIRClimate : public climate::Climate, public uart::UARTDevice, public Component {
 public:
  void dump_config() override;
  void setup() override;
  void loop() override;
  void control(const ClimateCall &call) override;
  ClimateTraits traits() override;

  // Setters
  void set_supported_modes(ClimateModeMask modes) { supported_modes_ = modes; }
  void set_supported_presets(ClimatePresetMask presets) { supported_presets_ = presets; }
  void set_supported_swing_modes(ClimateSwingModeMask modes) { supported_swing_modes_ = modes; }
  void set_custom_presets(const std::vector<std::string> &presets) { supported_custom_presets_ = presets; }
  void set_custom_fan_modes(const std::vector<std::string> &modes) { supported_custom_fan_modes_ = modes; }

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }

 protected:
  // Internal state members
  ClimateModeMask supported_modes_{};
  ClimatePresetMask supported_presets_{};
  ClimateSwingModeMask supported_swing_modes_{};
  std::vector<std::string> supported_custom_presets_{};
  std::vector<std::string> supported_custom_fan_modes_{};

  uart::UARTComponent *uart_{nullptr};

  // Parsed state variables
  ClimateMode mode_{ClimateMode::CLIMATE_MODE_OFF};
  ClimateFanMode fan_mode_{climate::CLIMATE_FAN_AUTO};
  float current_temperature_{0.0f};

  // Parsing methods for incoming UART data
  bool validate_message(const uint8_t *buf, size_t len);
  void parse_message(const uint8_t *buf, size_t len);

  // UART communication helpers
  void write_bytes(const uint8_t* data, size_t length);
  int read_bytes(uint8_t* buffer, size_t length, int timeout_ms);
  bool send_command(const std::vector<uint8_t> &command);
  bool read_response(std::vector<uint8_t> &response, int timeout_ms);
};

}  // namespace fujitsu_anywair
}  // namespace esphome
