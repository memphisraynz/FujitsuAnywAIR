#include "fujitsu_anywair.h"
#include "esphome/core/log.h"

static const char *TAG = "fujitsu_anywair.climate";

namespace esphome {
namespace fujitsu_anywair {

static uint8_t clamp_temperature(float temp) {
  if (temp < 16.0f) return 16;
  if (temp > 30.0f) return 30;
  return static_cast<uint8_t>(temp);
}

void FujitsuAnywAIRClimate::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Fujitsu AC Climate component");
}

void FujitsuAnywAIRClimate::loop() {
  static uint8_t buffer[kExpectedMsgLength];
  static size_t buffer_pos = 0;
  const size_t bufsize = sizeof(buffer);

  while (uart_->available() && buffer_pos < bufsize) {
    uint8_t byte;
    if (!uart_->read_byte(&byte)) {
      ESP_LOGE(TAG, "UART read error");
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
  // Example parsing, adjust offsets based on your protocol

  using namespace esphome::fujitsu_anywair;

  // Parse power (register or position example)
  if (buf[5] == static_cast<uint8_t>(Power::On)) {
    this->mode_ = climate::CLIMATE_MODE_HEAT;  // Example mapping
  } else {
    this->mode_ = climate::CLIMATE_MODE_OFF;
  }

  // Parse temperature (register or buffer offset)
  float temp = static_cast<float>(buf[6]);
  this->current_temperature = temp;

  // Parse mode from buffer byte
  uint8_t mode_byte = buf[7];
  Mode mode = static_cast<Mode>(mode_byte);
  switch (mode) {
    case Mode::Cool:
      this->mode_ = climate::CLIMATE_MODE_COOL;
      break;
    case Mode::Dry:
      this->mode_ = climate::CLIMATE_MODE_DRY;
      break;
    case Mode::Fan:
      this->mode_ = climate::CLIMATE_MODE_FAN_ONLY;
      break;
    case Mode::Heat:
      this->mode_ = climate::CLIMATE_MODE_HEAT;
      break;
    default:
      this->mode_ = climate::CLIMATE_MODE_OFF;
      break;
  }

  // Parse fan speed
  uint8_t fan_byte = buf[8];
  FanSpeed fan = static_cast<FanSpeed>(fan_byte);
  switch (fan) {
    case FanSpeed::Auto:
      this->fan_mode_ = climate::CLIMATE_FAN_AUTO;
      break;
    case FanSpeed::Quiet:
    case FanSpeed::Low:
      this->fan_mode_ = climate::CLIMATE_FAN_LOW;
      break;
    case FanSpeed::Medium:
      this->fan_mode_ = climate::CLIMATE_FAN_MEDIUM;
      break;
    case FanSpeed::High:
      this->fan_mode_ = climate::CLIMATE_FAN_HIGH;
      break;
    default:
      this->fan_mode_ = climate::CLIMATE_FAN_AUTO;
      break;
  }

  // Parse swing modes similarly...
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
