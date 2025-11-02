#include "fujitsu_anywair.h"

namespace esphome {
namespace fujitsu_anywair {
  // This will be called by ESPHome once to initialize the component.
  ESP_LOGCONFIG(TAG, "Setting up Fujitsu AC Climate component");
  // Here you could initiate UART communication or anything specific.
}

void FujitsuAnywAIRClimate::control(const climate::ClimateCall &call) {
  // This is called when there is a request to change HVAC mode, temperature, etc.
  // For now, just log what was requested.
  ESP_LOGD(TAG, "Received control command");
  if (call.get_target_temperature()) {
    ESP_LOGD(TAG, "Target temperature: %.1f", *call.get_target_temperature());
  }
  // Add HVAC control implementation here later.
}

climate::ClimateTraits FujitsuAnywAIRClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(true);
  traits.set_supports_target_temperature(true);
  traits.set_supports_operation_mode(climate::CLIMATE_OPERATION_MODE_OFF | climate::CLIMATE_OPERATION_MODE_HEAT);
  traits.min_temperature = 16.0f;
  traits.max_temperature = 30.0f;
  traits.precision = 0.5f;

  return traits;
}  // namespace fujitsu_anywair
}  // namespace esphome
