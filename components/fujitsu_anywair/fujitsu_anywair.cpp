#include "fujitsu_anywair.h"
#include "esphome/core/log.h"

static const char *TAG = "fujitsu_anywair.climate";

static constexpr size_t kExpectedMsgLength = 20;

namespace esphome {
namespace fujitsu_anywair {

// Helper conversions between ESPHome fan mode and your Fujitsu FanSpeed enum
FanSpeed to_fujitsu_fan_speed(climate::ClimateFanMode fan_mode) {
  switch (fan_mode) {
    case climate::CLIMATE_FAN_LOW:
      return FanSpeed::Low;
    case climate::CLIMATE_FAN_MEDIUM:
      return FanSpeed::Medium;
    case climate::CLIMATE_FAN_HIGH:
      return FanSpeed::High;
    case climate::CLIMATE_FAN_AUTO:
    default:
      return FanSpeed::Auto;
  }
}

climate::ClimateFanMode from_fujitsu_fan_speed(FanSpeed speed) {
  switch (speed) {
    case FanSpeed::Low:
      return climate::CLIMATE_FAN_LOW;
    case FanSpeed::Medium:
      return climate::CLIMATE_FAN_MEDIUM;
    case FanSpeed::High:
      return climate::CLIMATE_FAN_HIGH;
    case FanSpeed::Auto:
    default:
      return climate::CLIMATE_FAN_AUTO;
  }
}

static uint8_t clamp_temperature(float temp) {
  if (temp < 16.0f) return 16;
  if (temp > 30.0f) return 30;
  return static_cast<uint8_t>(temp);
}

void FujitsuAnywAIRClimate::set_supported_modes(const std::vector<climate::ClimateMode> &modes) {
  supported_modes_.clear();
  supported_modes_.insert(modes.begin(), modes.end());
}

void FujitsuAnywAIRClimate::set_supported_presets(const std::vector<climate::ClimatePreset> &presets) {
  supported_presets_.clear();
  supported_presets_.insert(presets.begin(), presets.end());
}

void FujitsuAnywAIRClimate::set_supported_swing_modes(const std::vector<climate::ClimateSwingMode> &modes) {
  supported_swing_modes_.clear();
  supported_swing_modes_.insert(modes.begin(), modes.end());
}

void FujitsuAnywAIRClimate::set_custom_fan_modes(const std::vector<std::string> &fan_modes) {
  supported_custom_fan_modes_ = fan_modes;
}

void FujitsuAnywAIRClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "Fujitsu AnywAIR Climate:");
  // Optionally log supported modes, presets, guitars etc.
}

void FujitsuAnywAIRClimate::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Fujitsu AnywAIR Climate");
}

void FujitsuAnywAIRClimate::loop() {
  static uint8_t buffer[kExpectedMsgLength];
  static size_t buffer_pos = 0;

  while (uart_ && uart_->available() && buffer_pos < kExpectedMsgLength) {
    uint8_t byte;
    if (!uart_->read_byte(&byte)) {
      break;
    }
    buffer[buffer_pos++] = byte;

    if (buffer_pos == kExpectedMsgLength) {
      if (validate_message(buffer, buffer_pos)) {
        parse_message(buffer, buffer_pos);
        publish_state();
      } else {
        ESP_LOGW(TAG, "Invalid Fujitsu message received");
      }
      buffer_pos = 0;
    }
  }
}

bool FujitsuAnywAIRClimate::validate_message(const uint8_t *buf, size_t len) {
  if (len != kExpectedMsgLength) {
    return false;
  }
  uint8_t sum = 0;
  for (size_t i = 0; i < len - 1; ++i) {
    sum += buf[i];
  }
  return (sum == buf[len - 1]);
}

void FujitsuAnywAIRClimate::parse_message(const uint8_t *buf, size_t len) {
  uint8_t mode_byte = buf[7];
  switch (mode_byte) {
    case 0x01: mode_ = climate::CLIMATE_MODE_COOL; break;
    case 0x02: mode_ = climate::CLIMATE_MODE_DRY; break;
    case 0x03: mode_ = climate::CLIMATE_MODE_FAN_ONLY; break;
    case 0x04: mode_ = climate::CLIMATE_MODE_HEAT; break;
    default: mode_ = climate::CLIMATE_MODE_OFF; break;
  }

  current_temperature_ = static_cast<float>(buf[6]);

  FanSpeed fan_speed = static_cast<FanSpeed>(buf[8]);
  fan_mode_ = from_fujitsu_fan_speed(fan_speed);
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
    switch (mode_opt.value()) {
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
  FanSpeed fujitsu_fan = to_fujitsu_fan_speed(call.get_fan_mode().value_or(climate::CLIMATE_FAN_AUTO));
  command_buffer.push_back(static_cast<uint8_t>(fujitsu_fan));

  // Vertical airflow / swing
  VerticalAirflow vertical_flow = VerticalAirflow::Position1;
  if (call.get_swing_mode() == climate::CLIMATE_SWING_VERTICAL) {
    vertical_flow = VerticalAirflow::Swing;
  }
  command_buffer.push_back(static_cast<uint8_t>(vertical_flow));

  // Horizontal airflow / swing
  HorizontalAirflow horizontal_flow = HorizontalAirflow::Position1;
  if (call.get_swing_mode() == climate::CLIMATE_SWING_HORIZONTAL) {
    horizontal_flow = HorizontalAirflow::Swing;
  }
  command_buffer.push_back(static_cast<uint8_t>(horizontal_flow));

  ESP_LOGD(TAG, "Sending command buffer:");
  for (auto b : command_buffer) {
    ESP_LOGD(TAG, " 0x%02X", b);
  }

  if (!send_command(command_buffer)) {
    ESP_LOGE(TAG, "Failed to send command");
  }
}

ClimateTraits FujitsuAnywAIRClimate::traits() {
  auto traits = ClimateTraits();

  traits.set_supported_modes(supported_modes_);
  traits.set_supported_presets(supported_presets_);
  traits.set_supported_swing_modes(supported_swing_modes_);
  traits.set_supported_fan_modes(supported_fan_modes_);

  traits.set_visual_min_temperature(16.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_temperature_step(1.0f);

  return traits;
}

void FujitsuAnywAIRClimate::write_bytes(const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    uart_->write_byte(data[i]);
  }
  uart_->flush();
}

int FujitsuAnywAIRClimate::read_bytes(uint8_t* buffer, size_t length, int timeout_ms) {
  int read = 0;
  uint32_t start = millis();
  while (read < static_cast<int>(length) && (millis() - start) < static_cast<uint32_t>(timeout_ms)) {
    if (uart_->available()) {
      uint8_t b;
      if (uart_->read_byte(&b)) {
        buffer[read++] = b;
      }
    }
  }
  return read;
}

bool FujitsuAnywAIRClimate::send_command(const std::vector<uint8_t> &command) {
  write_bytes(command.data(), command.size());
  return true;
}

bool FujitsuAnywAIRClimate::read_response(std::vector<uint8_t> &response, int timeout_ms) {
  constexpr int RESPONSE_LENGTH = kExpectedMsgLength;
  response.resize(RESPONSE_LENGTH);
  int read = read_bytes(response.data(), RESPONSE_LENGTH, timeout_ms);
  return read == RESPONSE_LENGTH;
}

}  // namespace fujitsu_anywair
}  // namespace esphome
