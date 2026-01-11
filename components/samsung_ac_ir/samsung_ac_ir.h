#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/core/log.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Samsung.h>

namespace esphome {
namespace samsung_ac_ir {

class SamsungAcIrClimate : public climate::Climate, public Component {
 public:
  void set_ir_pin(uint8_t pin) { this->ir_pin_ = pin; }
  void set_supports_swing_horizontal(bool v) { this->supports_swing_horizontal_ = v; }
  void set_supports_swing_vertical(bool v) { this->supports_swing_vertical_ = v; }

  void setup() override;
  void loop() override {}

  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

 protected:
  void send_ir_();
  void apply_swing_();

  static const char *const TAG;

  uint8_t ir_pin_{4};
  bool supports_swing_horizontal_{true};
  bool supports_swing_vertical_{true};

  IRsend irsend_{4};
  IRSamsungAc ac_{4};

  bool power_on_{false};
};

}  // namespace samsung_ac_ir
}  // namespace esphome