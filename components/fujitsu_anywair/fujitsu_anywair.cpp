#include "fujitsu_anywair.h"
#include "esphome/components/climate/climate.h"

static const char *TAG = "fujitsu_anywair.climate";

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
  // Add control logic
}

climate::ClimateTraits FujitsuAnywAIRClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(true);
  // There is no set_supports_target_temperature; remove this line

  // Use set for supported modes
  std::set<climate::ClimateMode> modes = {
    climate::ClimateMode::CLIMATE_MODE_OFF,
    climate::ClimateMode::CLIMATE_MODE_HEAT
  };
  if (supports_cool_) {
    modes.insert(climate::ClimateMode::CLIMATE_MODE_COOL);
  }
  traits.set_supported_modes(std::move(modes));

  // Use setters for visual temp ranges and precision
  traits.set_visual_min_temperature(16.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_precision(0.5f);

  return traits;
}

}  // namespace fujitsu_anywair
}  // namespace esphome
