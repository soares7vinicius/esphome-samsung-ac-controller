#include "samsung_ac_ir.h"

#include <cmath>

namespace esphome {
namespace samsung_ac_ir {

const char *const SamsungAcIrClimate::TAG = "samsung_ac";

template <typename T>
static auto has_setSwingH_impl(int) -> decltype(std::declval<T &>().setSwingH(true), std::true_type{});
template <typename T>
static auto has_setSwingH_impl(...) -> std::false_type;
template <typename T>
static constexpr bool has_setSwingH_v = decltype(has_setSwingH_impl<T>(0))::value;

void SamsungAcIrClimate::setup() {
  this->irsend_ = IRsend(this->ir_pin_);
  this->ac_ = IRSamsungAc(this->ir_pin_);

  this->irsend_.begin();
  this->ac_.begin();

  // Defaults
  this->mode = climate::CLIMATE_MODE_OFF;
  this->target_temperature = 24.0f;
  this->fan_mode = climate::CLIMATE_FAN_HIGH;
  this->swing_mode = climate::CLIMATE_SWING_BOTH;

  this->power_on_ = false;

  ESP_LOGI(TAG, "Initialized on GPIO%u", this->ir_pin_);
  this->publish_state();
}

climate::ClimateTraits SamsungAcIrClimate::traits() {
  auto t = climate::ClimateTraits();

  t.set_supports_current_temperature(false);  // você pode integrar um sensor depois
  t.set_supports_two_point_target_temperature(false);

  t.set_visual_min_temperature(16);
  t.set_visual_max_temperature(30);
  t.set_visual_temperature_step(1.0f);

  t.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_DRY,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_AUTO,
  });

  t.set_supported_fan_modes({
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
  });

  // Swing modes expostos conforme flags
  climate::ClimateSwingModeMask swings;
  swings.insert(climate::CLIMATE_SWING_OFF);

  if (this->supports_swing_vertical_) swings.insert(climate::CLIMATE_SWING_VERTICAL);
  if (this->supports_swing_horizontal_) swings.insert(climate::CLIMATE_SWING_HORIZONTAL);
  if (this->supports_swing_vertical_ && this->supports_swing_horizontal_) swings.insert(climate::CLIMATE_SWING_BOTH);

  t.set_supported_swing_modes(swings);

  return t;
}

void SamsungAcIrClimate::control(const climate::ClimateCall &call) {
  bool changed = false;

  if (call.get_mode().has_value()) {
    this->mode = *call.get_mode();
    changed = true;
  }
  if (call.get_target_temperature().has_value()) {
    this->target_temperature = *call.get_target_temperature();
    changed = true;
  }
  if (call.get_fan_mode().has_value()) {
    this->fan_mode = *call.get_fan_mode();
    changed = true;
  }
  if (call.get_swing_mode().has_value()) {
    this->swing_mode = *call.get_swing_mode();
    changed = true;
  }

  if (!changed) return;

  this->send_ir_();
  this->publish_state();
}

void SamsungAcIrClimate::apply_swing_() {
  auto sw = this->swing_mode;

  bool v = (sw == climate::CLIMATE_SWING_VERTICAL) || (sw == climate::CLIMATE_SWING_BOTH);
  bool h = (sw == climate::CLIMATE_SWING_HORIZONTAL) || (sw == climate::CLIMATE_SWING_BOTH);

  // Vertical
  if (this->supports_swing_vertical_) {
    this->ac_.setSwing(v);
  }

  // Horizontal (depende da versão / suporte)
  if (this->supports_swing_horizontal_) {
    if constexpr (has_setSwingH_v<IRSamsungAc>) {
      this->ac_.setSwingH(h);
    } else {
      if (h) {
        ESP_LOGW(TAG, "Swing horizontal requested but IRSamsungAc::setSwingH not available in this IRremoteESP8266 build");
      }
    }
  }
}

void SamsungAcIrClimate::send_ir_() {
  // Power
  this->power_on_ = (this->mode != climate::CLIMATE_MODE_OFF);
  this->ac_.setPower(this->power_on_);

  // Mode
  if (this->power_on_) {
    switch (this->mode) {
      case climate::CLIMATE_MODE_COOL:
        this->ac_.setMode(kSamsungAcCool);
        break;
      case climate::CLIMATE_MODE_HEAT:
        this->ac_.setMode(kSamsungAcHeat);
        break;
      case climate::CLIMATE_MODE_DRY:
        this->ac_.setMode(kSamsungAcDry);
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
        this->ac_.setMode(kSamsungAcFan);
        break;
      case climate::CLIMATE_MODE_AUTO:
        this->ac_.setMode(kSamsungAcAuto);
        break;
      default:
        this->ac_.setMode(kSamsungAcCool);
        break;
    }
  }

  // Temp
  int t = (int) lroundf(this->target_temperature);
  if (t < 16) t = 16;
  if (t > 30) t = 30;
  this->ac_.setTemp(t);

  // Fan
  auto fan = this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO);
  switch (fan) {
    case climate::CLIMATE_FAN_LOW:
      this->ac_.setFan(kSamsungAcFanLow);
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      this->ac_.setFan(kSamsungAcFanMed);
      break;
    case climate::CLIMATE_FAN_HIGH:
      this->ac_.setFan(kSamsungAcFanHigh);
      break;
    case climate::CLIMATE_FAN_AUTO:
    default:
      this->ac_.setFan(kSamsungAcFanAuto);
      break;
  }

  // Swing
  this->apply_swing_();

  ESP_LOGI(TAG, "Send IR: power=%s mode=%d temp=%d fan=%d swing=%d",
           this->power_on_ ? "on" : "off",
           (int) this->mode,
           t,
           (int) fan,
           (int) this->swing_mode);

  // AC costuma ser mais confiável com 2 envios
  this->ac_.send();
  delay(120);
  this->ac_.send();
}

}  // namespace samsung_ac_ir
}  // namespace esphome
