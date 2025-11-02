#include "fujitsu_anywair.h"
#include "esphome/core/log.h"

static const char *TAG = "fujitsu_anywair.climate";

namespace esphome {
namespace fujitsu_anywair {

void FujitsuAnywAIRClimate::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Fujitsu AC Climate component");
}

void FujitsuAnywAIRClimate::control(const climate::ClimateCall &call) {
  ESP_LOGD(TAG, "Received control command");

  std::vector<uint8_t> command_buffer;

  // Power
  auto mode_opt = call.get_mode();
  if (!mode_opt || mode_opt.value() == climate::CLIMATE_MODE_OFF) {
    command_buffer.push_back(static_cast<uint8_t>(Power::Off));
  } else {
    command_buffer.push_back(static_cast<uint8_t>(Power::On));
  }

  // Mode
  Mode fujitsu_mode = Mode::Auto;
  if (mode_opt) {
    switch(mode_opt.value()) {
      case climate::CLIMATE_MODE_COOL:
        fujitsu_mode = Mode::Cool;
        break;
      case climate::CLIMATE_MODE_DRY:
        fujitsu_mode = Mode::Dry;
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
        fujitsu_mode = Mode::Fan;
        break;
      case climate::CLIMATE_MODE_HEAT:
        fujitsu_mode = Mode::Heat;
        break;
      default:
        fujitsu_mode = Mode::Auto;
        break;
    }
  }
  command_buffer.push_back(static_cast<uint8_t>(fujitsu_mode));

  // Target temperature
  if (call.get_target_temperature()) {
    command_buffer.push_back(clamp_temperature(*call.get_target_temperature()));
  } else {
    command_buffer.push_back(0);
  }

  // Fan speed
  climate::ClimateFanMode fan = call.get_fan_mode().value_or(climate::CLIMATE_FAN_AUTO);
  FanSpeed fujitsu_fan = FanSpeed::Auto;
  switch(fan) {
    case climate::CLIMATE_FAN_LOW:
      fujitsu_fan = FanSpeed::Low;
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      fujitsu_fan = FanSpeed::Medium;
      break;
    case climate::CLIMATE_FAN_HIGH:
      fujitsu_fan = FanSpeed::High;
      break;
    case climate::CLIMATE_FAN_AUTO:
    default:
      fujitsu_fan = FanSpeed::Auto;
      break;
  }
  command_buffer.push_back(static_cast<uint8_t>(fujitsu_fan));

  // Vertical airflow / swing - example uses VerticalAirflow enums
  // Here, you can map swing/position from climate extras or presets if you have them
  VerticalAirflow vertical_flow = VerticalAirflow::Position1;  // default
  // Example: map swing mode flag on/off
  if (call.get_swing_mode() == climate::CLIMATE_SWING_VERTICAL) {
    vertical_flow = VerticalAirflow::Swing;
  }
  command_buffer.push_back(static_cast<uint8_t>(vertical_flow));

  // Horizontal airflow / swing - similar mapping
  HorizontalAirflow horizontal_flow = HorizontalAirflow::Position1;  // default
  // Example: map swing mode flag on/off
  if (call.get_swing_mode() == climate::CLIMATE_SWING_HORIZONTAL) {
    horizontal_flow = HorizontalAirflow::Swing;
  }
  command_buffer.push_back(static_cast<uint8_t>(horizontal_flow));

  // Add additional flags (powerful, economy_mode, etc.) if desired using similar pattern

  ESP_LOGD(TAG, "Sending command buffer:");
  for (auto b : command_buffer) {
    ESP_LOGD(TAG, " 0x%02X", b);
  }

  // Send the command buffer over UART
  if (!this->send_command(command_buffer)) {
    ESP_LOGE(TAG, "Failed to send command");
  }
}

climate::ClimateTraits FujitsuAnywAIRClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);

  std::set<climate::ClimateMode> modes = {
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_HEAT,
  };

  if (supports_cool_) {
    modes.insert(climate::CLIMATE_MODE_COOL);
  }

  traits.set_supported_modes(std::move(modes));
  traits.set_visual_min_temperature(16.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_temperature_step(1.0f);

  return traits;
}

// Communication helper implementations:

void FujitsuAnywAIRClimate::write_bytes(const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    uart_->write_byte(data[i]);
  }
  uart_->flush();
}

int FujitsuAnywAIRClimate::read_bytes(uint8_t* buffer, size_t length, int timeout_ms) {
  int read = 0;
  uint32_t start = millis();  // uptime in ms since boot
  while (read < static_cast<int>(length) && (millis() - start) < static_cast<uint32_t>(timeout_ms)) {
    if (uart_->available()) {
      uint8_t b;
      if (uart_->read_byte(&b)) {   // read_byte takes pointer and returns bool success
        buffer[read++] = b;
      }
    }
  }
  return read;
}

bool FujitsuAnywAIRClimate::send_command(const std::vector<uint8_t> &command) {
  write_bytes(command.data(), command.size());
  // Add error checking or response validation as needed
  return true;
}

bool FujitsuAnywAIRClimate::read_response(std::vector<uint8_t> &response, int timeout_ms) {
  constexpr int RESPONSE_LENGTH = 20;  // Adjust based on your Fujitsu protocol
  response.resize(RESPONSE_LENGTH);
  int read = read_bytes(response.data(), RESPONSE_LENGTH, timeout_ms);
  return read == RESPONSE_LENGTH;
}

}  // namespace fujitsu_anywair
}  // namespace esphome
