#include "fujitsu_ac.h"

namespace esphome {
namespace fujitsu_anywair {

void FujitsuAnywAIRClimate::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Fujitsu AC Climate component");
}

void FujitsuAnywAIRClimate::control(const climate::ClimateCall &call) {
  ESP_LOGD(TAG, "Received control command");
  if (call.get_target_temperature()) {
    ESP_LOGD(TAG, "Target temperature: %.1f", *call.get_target_temperature());
  }
  // Implement control logic here
}

climate::ClimateTraits FujitsuAnywAIRClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_target_temperature(true);
  traits.set_supports_operation_mode(
      climate::CLIMATE_OPERATION_MODE_OFF | climate::CLIMATE_OPERATION_MODE_HEAT |
      (supports_cool_ ? climate::CLIMATE_OPERATION_MODE_COOL : 0));
  traits.min_temperature = 16.0f;
  traits.max_temperature = 30.0f;
  traits.precision = 0.5f;

  return traits;
}

}  // namespace fujitsu_anywair
}  // namespace esphome
