#pragma once

#ifdef USE_ARDUINO

#include "esphome/core/macros.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/color.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/light/esp_color_correction.h"
#include "esphome/components/light/esp_hsv_color.h"
#include "NeoPixelBus.h"

namespace esphome {
namespace lsc_rgbww {

class LSCRGBWWLightOutput : public light::LightOutput {
 public:
  // void set_red(uint8_t *red) { red_ = red; }
  // void set_green(uint8_t *green) { green_ = green; }
  // void set_blue(uint8_t *blue) { blue_ = blue; }
  // void set_cold_white(uint8_t *cold_white) { cold_white_ = cold_white; }
  // void set_warm_white(uint8_t *warm_white) { warm_white_ = warm_white; }
  void config(uint16_t count_pixels, uint8_t pin) {
    this->strip = new NeoPixelBus<NeoGrbcwxFeature, NeoWs2812xMethod>(count_pixels, pin);
    this->strip->Begin();  // Example initialization method
    this->strip->Show();
  }
  void set_cold_white_temperature(float cold_white_temperature) { cold_white_temperature_ = cold_white_temperature; }
  void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }
  void set_constant_brightness(bool constant_brightness) { constant_brightness_ = constant_brightness; }
  void set_color_interlock(bool color_interlock) { color_interlock_ = color_interlock; }
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    if (this->color_interlock_) {
      traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLD_WARM_WHITE});
    } else {
      traits.set_supported_color_modes({light::ColorMode::RGB_COLD_WARM_WHITE});
    }
    traits.set_min_mireds(this->cold_white_temperature_);
    traits.set_max_mireds(this->warm_white_temperature_);
    return traits;
  }
  void write_state(light::LightState *state) override {
    float red, green, blue, cwhite, wwhite;
    state->current_values_as_rgbww(&red, &green, &blue, &cwhite, &wwhite, this->constant_brightness_);
    RgbwwColor color = RgbwwColor(red, green, blue, cwhite, wwhite);
    this->strip->SetPixelColor(0, color);
    this->strip->Show();
  }

 protected:
  NeoPixelBus<NeoGrbcwxFeature, NeoWs2812xMethod> *strip;
  uint8_t *red_;
  uint8_t *green_;
  uint8_t *blue_;
  uint8_t *cold_white_;
  uint8_t *warm_white_;
  float cold_white_temperature_{0};
  float warm_white_temperature_{0};
  bool constant_brightness_;
  bool color_interlock_{false};
};

}  // namespace lsc_rgbww
}  // namespace esphome
#endif  // USE_ARDUINO
