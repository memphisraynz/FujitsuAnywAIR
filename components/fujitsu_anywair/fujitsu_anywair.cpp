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

  // Here, you will build the command bytes depending on the requested climate settings.
  // This is a placeholder example. You will need to map climate::ClimateCall data
  // to the Fujitsu protocol bytes.
  std::vector<uint8_t> command = {0x00, 0x00, 0x00};  // Replace with actual command bytes

  if (this->send_command(command)) {
    ESP_LOGD(TAG, "Command sent successfully");
  } else {
    ESP_LOGE(TAG, "Failed to send command");
  }
  
  // Optionally read response and handle status updates
  // std::vector<uint8_t> response;
  // if (this->read_response(response)) {
  //   ESP_LOGD(TAG, "Response received");
  // }
}

climate::ClimateTraits FujitsuAnywAIRClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);

  std::set<climate::ClimateMode> modes = {
      climate::ClimateMode::CLIMATE_MODE_OFF,
      climate::ClimateMode::CLIMATE_MODE_HEAT,
  };
  if (supports_cool_) {
    modes.insert(climate::ClimateMode::CLIMATE_MODE_COOL);
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
  uint32_t start = esp_log_uptime_millis();  // uptime in ms since boot
  while (read < static_cast<int>(length) && (esp_log_uptime_millis() - start) < static_cast<uint32_t>(timeout_ms)) {
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
