#include "esphome.h"
#include <Adafruit_NeoPixel.h>

class CustomAdressableRGBCCT : public Component, public LightOutput {
 public:
  CustomAdressableRGBCCT(uint16_t num_sections, uint8_t data_pin)
      : num_sections_(num_sections), strip_(Adafruit_NeoPixel(num_sections * 2, data_pin, NEO_GRBW + NEO_KHZ800)) {}

  void setup() override {
    // Initialize Neopixel strip
    strip_.begin();
    strip_.show();  // Clear the strip
  }

  LightTraits get_traits() override {
    auto traits = LightTraits();
    traits.set_supports_rgb(true);
    traits.set_supports_white_value(true);
    return traits;
  }

  void write_state(LightState *state) override {
    float red, green, blue, white;
    state->current_values_as_rgbw(&red, &green, &blue, &white);

    for (uint16_t i = 0; i < num_sections_; i++) {
      // Map RGB values to the first IC of each section
      uint16_t rgb_index = 2 * i;  // First IC
      strip_.setPixelColor(rgb_index, strip_.Color(
        red * 255, green * 255, blue * 255));

      // Map White values to the second IC of each section
      uint16_t ww_cw_index = 2 * i + 1;  // Second IC
      strip_.setPixelColor(ww_cw_index, strip_.Color(
        0, 0, 0, white * 255));
    }

    strip_.show();  // Push the updated data to the strip
  }

 private:
  uint16_t num_sections_;
  Adafruit_NeoPixel strip_;
};

