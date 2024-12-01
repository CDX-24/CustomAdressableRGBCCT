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
namespace neopixelbus {

enum class ESPNeoPixelOrder {
  GBWR = 0b11000110,
  GBRW = 0b10000111,
  GBR = 0b10000111,
  GWBR = 0b11001001,
  GRBW = 0b01001011,
  GRB = 0b01001011,
  GWRB = 0b10001101,
  GRWB = 0b01001110,
  BGWR = 0b11010010,
  BGRW = 0b10010011,
  BGR = 0b10010011,
  WGBR = 0b11011000,
  RGBW = 0b00011011,
  RGBWW = 0b00011111,
  BGRWW = 0b10010111,
  BRGWW = 0b01100111,
  RGB = 0b00011011,
  WGRB = 0b10011100,
  RGWB = 0b00011110,
  BWGR = 0b11100001,
  BRGW = 0b01100011,
  BRG = 0b01100011,
  WBGR = 0b11100100,
  RBGW = 0b00100111,
  RBG = 0b00100111,
  WRGB = 0b01101100,
  RWGB = 0b00101101,
  BWRG = 0b10110001,
  BRWG = 0b01110010,
  WBRG = 0b10110100,
  RBWG = 0b00110110,
  WRBG = 0b01111000,
  RWBG = 0b00111001,
};

template<typename T_METHOD, typename T_COLOR_FEATURE>
class NeoPixelBusLightOutputBase : public light::AddressableLight {
 public:
  NeoPixelBus<T_COLOR_FEATURE, T_METHOD> *get_controller() const { return this->controller_; }

  void clear_effect_data() override {
    for (int i = 0; i < this->size(); i++)
      this->effect_data_[i] = 0;
  }

  /// Add some LEDS, can only be called once.
  void add_leds(uint16_t count_pixels, uint8_t pin) {
    this->add_leds(new NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(count_pixels, pin));
  }
  void add_leds(uint16_t count_pixels, uint8_t pin_clock, uint8_t pin_data) {
    this->add_leds(new NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(count_pixels, pin_clock, pin_data));
  }
  void add_leds(uint16_t count_pixels) { this->add_leds(new NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(count_pixels)); }
  void add_leds(NeoPixelBus<T_COLOR_FEATURE, T_METHOD> *controller) {
    this->controller_ = controller;
    // controller gets initialised in setup() - avoid calling twice (crashes with RMT)
    // this->controller_->Begin();
  }

  // ========== INTERNAL METHODS ==========
  void setup() override {
    for (int i = 0; i < this->size(); i++) {
      (*this)[i] = Color(0, 0, 0, 0);
    }

    this->effect_data_ = new uint8_t[this->size()];  // NOLINT
    this->controller_->Begin();
  }

  void write_state(light::LightState *state) override {
    this->mark_shown_();
    this->controller_->Dirty();

    this->controller_->Show();
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  int32_t size() const override { return this->controller_->PixelCount(); }

  void set_pixel_order(ESPNeoPixelOrder order) {
    uint8_t u_order = static_cast<uint8_t>(order);
    this->rgb_offsets_[0] = (u_order >> 6) & 0b11;
    this->rgb_offsets_[1] = (u_order >> 4) & 0b11;
    this->rgb_offsets_[2] = (u_order >> 2) & 0b11;
    this->rgb_offsets_[3] = (u_order >> 0) & 0b11;
  }

 protected:
  NeoPixelBus<T_COLOR_FEATURE, T_METHOD> *controller_{nullptr};
  uint8_t *effect_data_{nullptr};
  uint8_t rgb_offsets_[4]{0, 1, 2, 3};
};

template<typename T_METHOD, typename T_COLOR_FEATURE = NeoRgbFeature>
class NeoPixelRGBLightOutput : public NeoPixelBusLightOutputBase<T_METHOD, T_COLOR_FEATURE> {
 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB});
    return traits;
  }

 protected:
  light::ESPColorView get_view_internal(int32_t index) const override {  // NOLINT
    uint8_t *base = this->controller_->Pixels() + 3ULL * index;
    return light::ESPColorView(base + this->rgb_offsets_[0], base + this->rgb_offsets_[1], base + this->rgb_offsets_[2],
                               nullptr, this->effect_data_ + index, &this->correction_);
  }
};

template<typename T_METHOD, typename T_COLOR_FEATURE = NeoRgbwFeature>
class NeoPixelRGBWLightOutput : public NeoPixelBusLightOutputBase<T_METHOD, T_COLOR_FEATURE> {
 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB_WHITE});
    return traits;
  }

 protected:
  light::ESPColorView get_view_internal(int32_t index) const override {  // NOLINT
    uint8_t *base = this->controller_->Pixels() + 4ULL * index;
    return light::ESPColorView(base + this->rgb_offsets_[0], base + this->rgb_offsets_[1], base + this->rgb_offsets_[2],
                               base + this->rgb_offsets_[3], this->effect_data_ + index, &this->correction_);
  }
};
template<typename T_METHOD, typename T_COLOR_FEATURE = NeoRgbwwFeature>
class NeoPixelRGBWWLightOutput : public NeoPixelBusLightOutputBase<T_METHOD, T_COLOR_FEATURE> {
 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB_COLD_WARM_WHITE});
    traits.set_min_mireds("2700");
    traits.set_max_mireds("6000");
    return traits;
  }

 protected:
  light_rgbww::ESPColorView get_view_internal(int32_t index) const override {  // NOLINT
    uint8_t *base = this->controller_->Pixels() + 5ULL * index;
    return light_rgbww::ESPColorView(base + this->rgb_offsets_[0], base + this->rgb_offsets_[1], base + this->rgb_offsets_[2],
                               base + this->rgb_offsets_[3], base + this->rgb_offsets_[4], this->effect_data_ + index, &this->correction_);
  }
};

}  // namespace neopixelbus

namespace light_rgbww {
class ESPColorSettable {
 public:
  virtual void set(const Color &color) = 0;
  virtual void set_red(uint8_t red) = 0;
  virtual void set_green(uint8_t green) = 0;
  virtual void set_blue(uint8_t blue) = 0;
  virtual void set_white(uint8_t white) = 0;
  virtual void set_c_white(uint8_t c_white) = 0;
  virtual void set_w_white(uint8_t w_white) = 0;
  virtual void set_effect_data(uint8_t effect_data) = 0;
  virtual void fade_to_white(uint8_t amnt) = 0;
  virtual void fade_to_black(uint8_t amnt) = 0;
  virtual void lighten(uint8_t delta) = 0;
  virtual void darken(uint8_t delta) = 0;
  void set(const light::ESPHSVColor &color) { this->set_hsv(color); }
  void set_hsv(const light::ESPHSVColor &color) {
    Color rgb = color.to_rgb();
    this->set_rgb(rgb.r, rgb.g, rgb.b);
  }
  void set_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    this->set_red(red);
    this->set_green(green);
    this->set_blue(blue);
  }
  void set_rgbw(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    this->set_rgb(red, green, blue);
    this->set_white(white);
  }
  void set_rgbww(uint8_t red, uint8_t green, uint8_t blue, uint8_t c_white, uint8_t w_white) {
    this->set_rgb(red, green, blue);
    this->set_w_white(w_white);
    this->set_c_white(c_white);
  }
};

class ESPColorView : public ESPColorSettable {
 public:
  ESPColorView(uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *c_white, uint8_t *w_white, uint8_t *effect_data,
               const light::ESPColorCorrection *color_correction)
      : red_(red),
        green_(green),
        blue_(blue),
        c_white_(c_white),
        w_white_(w_white),
        effect_data_(effect_data),
        color_correction_(color_correction) {}
  ESPColorView &operator=(const Color &rhs) {
    this->set(rhs);
    return *this;
  }
  ESPColorView &operator=(const light::ESPHSVColor &rhs) {
    this->set_hsv(rhs);
    return *this;
  }
  void set(const Color &color) override { this->set_rgbw(color.r, color.g, color.b, color.w); }
  void set_red(uint8_t red) override { *this->red_ = this->color_correction_->color_correct_red(red); }
  void set_green(uint8_t green) override { *this->green_ = this->color_correction_->color_correct_green(green); }
  void set_blue(uint8_t blue) override { *this->blue_ = this->color_correction_->color_correct_blue(blue); }
  void set_white(uint8_t white) override {
    if (this->white_ == nullptr)
      return;
    *this->white_ = this->color_correction_->color_correct_white(white);
  }
  void set_effect_data(uint8_t effect_data) override {
    if (this->effect_data_ == nullptr)
      return;
    *this->effect_data_ = effect_data;
  }
  void fade_to_white(uint8_t amnt) override { this->set(this->get().fade_to_white(amnt)); }
  void fade_to_black(uint8_t amnt) override { this->set(this->get().fade_to_black(amnt)); }
  void lighten(uint8_t delta) override { this->set(this->get().lighten(delta)); }
  void darken(uint8_t delta) override { this->set(this->get().darken(delta)); }
  Color get() const { return Color(this->get_red(), this->get_green(), this->get_blue(), this->get_c_white()); }
  uint8_t get_red() const { return this->color_correction_->color_uncorrect_red(*this->red_); }
  uint8_t get_red_raw() const { return *this->red_; }
  uint8_t get_green() const { return this->color_correction_->color_uncorrect_green(*this->green_); }
  uint8_t get_green_raw() const { return *this->green_; }
  uint8_t get_blue() const { return this->color_correction_->color_uncorrect_blue(*this->blue_); }
  uint8_t get_blue_raw() const { return *this->blue_; }
  uint8_t get_w_white() const {
    if (this->w_white_ == nullptr)
      return 0;
    return this->color_correction_->color_uncorrect_white(*this->w_white_);
  }
  uint8_t get_w_white_raw() const {
    if (this->w_white_ == nullptr)
      return 0;
    return *this->w_white_;
  }
  uint8_t get_c_white() const {
    if (this->c_white_ == nullptr)
      return 0;
    return this->color_correction_->color_uncorrect_white(*this->c_white_);
  }
  uint8_t get_c_white_raw() const {
    if (this->c_white_ == nullptr)
      return 0;
    return *this->c_white_;
  }
  uint8_t get_effect_data() const {
    if (this->effect_data_ == nullptr)
      return 0;
    return *this->effect_data_;
  }
  void raw_set_color_correction(const light::ESPColorCorrection *color_correction) {
    this->color_correction_ = color_correction;
  }

 protected:
  uint8_t *const red_;
  uint8_t *const green_;
  uint8_t *const blue_;
  uint8_t *const c_white_;
  uint8_t *const w_white_;
  uint8_t *const effect_data_;
  const light::ESPColorCorrection *color_correction_;
};
}  // namespace light
}  // namespace esphome
#endif  // USE_ARDUINO
