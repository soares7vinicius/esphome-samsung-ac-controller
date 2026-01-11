#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Samsung.h>

using namespace esphome;

class SamsungAcClimate : public Component, public climate::Climate {
 public:
  explicit SamsungAcClimate(uint8_t ir_pin)
      : ir_pin_(ir_pin), irsend_(ir_pin), ac_(ir_pin) {}

  void setup() override {
    irsend_.begin();
    ac_.begin();

    // Defaults sensatos
    this->mode = climate::CLIMATE_MODE_OFF;
    this->target_temperature = 24.0f;
    this->fan_mode = climate::CLIMATE_FAN_HIGH;
    this->swing_mode = climate::CLIMATE_SWING_BOTH;

    publish_state();
  }

  climate::ClimateTraits traits() override {
    auto t = climate::ClimateTraits();
    t.set_supports_current_temperature(false);     // você vai ter sensor separado
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

    t.set_supported_swing_modes({
      climate::CLIMATE_SWING_OFF,
      climate::CLIMATE_SWING_VERTICAL,
      climate::CLIMATE_SWING_HORIZONTAL,
      climate::CLIMATE_SWING_BOTH,
    });

    return t;
  }

  void control(const climate::ClimateCall &call) override {
    bool dirty = false;

    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
      dirty = true;
    }
    if (call.get_target_temperature().has_value()) {
      this->target_temperature = *call.get_target_temperature();
      dirty = true;
    }
    if (call.get_fan_mode().has_value()) {
      this->fan_mode = *call.get_fan_mode();
      dirty = true;
    }
    if (call.get_swing_mode().has_value()) {
      this->swing_mode = *call.get_swing_mode();
      dirty = true;
    }

    if (dirty) {
      this->send_ir_();
      this->publish_state();
    }
  }

 private:
  void send_ir_() {
    // Power
    bool power_on = (this->mode != climate::CLIMATE_MODE_OFF);
    ac_.setPower(power_on);

    // Mode mapping
    if (!power_on) {
      // Mesmo off, alguns aparelhos aceitam frame completo; manter defaults.
    } else {
      switch (this->mode) {
        case climate::CLIMATE_MODE_COOL: ac_.setMode(kSamsungAcCool); break;
        case climate::CLIMATE_MODE_HEAT: ac_.setMode(kSamsungAcHeat); break;
        case climate::CLIMATE_MODE_DRY:  ac_.setMode(kSamsungAcDry); break;
        case climate::CLIMATE_MODE_FAN_ONLY: ac_.setMode(kSamsungAcFan); break;
        case climate::CLIMATE_MODE_AUTO: ac_.setMode(kSamsungAcAuto); break;
        default: ac_.setMode(kSamsungAcCool); break;
      }
    }

    // Temp
    int t = (int) lroundf(this->target_temperature);
    if (t < 16) t = 16;
    if (t > 30) t = 30;
    ac_.setTemp(t);

    // Fan
    switch (this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO)) {
      case climate::CLIMATE_FAN_LOW:    ac_.setFan(kSamsungAcFanLow); break;
      case climate::CLIMATE_FAN_MEDIUM: ac_.setFan(kSamsungAcFanMed); break;
      case climate::CLIMATE_FAN_HIGH:   ac_.setFan(kSamsungAcFanHigh); break;
      case climate::CLIMATE_FAN_AUTO:
      default:                          ac_.setFan(kSamsungAcFanAuto); break;
    }

    // Swing
    // Observação: nem todo modelo suporta H + V separados. Mantive ambos quando possível.
    auto sw = this->swing_mode.value_or(climate::CLIMATE_SWING_OFF);
    ac_.setSwing(sw == climate::CLIMATE_SWING_VERTICAL || sw == climate::CLIMATE_SWING_BOTH);
    // Horizontal em Samsung AC pode ser feature separada (varia). Se a sua lib suportar, a gente liga aqui.
    // Ex.: ac_.setSwingH(...); (depende da versão)

    ESP_LOGI("samsung_ac", "Send: power=%d mode=%d temp=%d fan=%d swing=%d",
         power_on, (int)this->mode, t, (int)this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO),
         (int)this->swing_mode.value_or(climate::CLIMATE_SWING_OFF));

    // Envia 2x (muitos ACs são mais confiáveis assim)
    ac_.send();
    delay(120);
    ac_.send();
  }

  uint8_t ir_pin_;
  IRsend irsend_;
  IRSamsungAc ac_;
};