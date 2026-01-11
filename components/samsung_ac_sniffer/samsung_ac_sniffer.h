#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include <string>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRac.h>

namespace esphome {
namespace samsung_ac_sniffer {

class SamsungAcSniffer : public Component {
 public:
  void set_rx_pin(uint8_t pin) { this->rx_pin_ = pin; }
  void set_dump_raw(bool v) { this->dump_raw_ = v; }
  void set_dump_state(bool v) { this->dump_state_ = v; }
  void set_dump_desc(bool v) { this->dump_desc_ = v; }
  void set_rate_limit_ms(uint32_t v) { this->rate_limit_ms_ = v; }
  void set_buffer_size(uint16_t v) { this->buffer_size_override_ = v; }
  void set_timeout_ms(uint16_t v) { this->timeout_override_ms_ = v; }
  void set_tolerance(uint8_t v) { this->tolerance_override_ = v; }
  void set_min_unknown_size(uint16_t v) { this->min_unknown_size_override_ = v; }

  void setup() override;
  void loop() override;

 protected:
  void log_result_();
  std::string format_state_hex_(size_t nbytes) const;
  void log_state_array_(size_t nbytes) const;

  static const char *const TAG;

  uint8_t rx_pin_{21};
  bool dump_raw_{true};
  bool dump_state_{true};
  bool dump_desc_{true};
  uint32_t rate_limit_ms_{500};
  uint16_t buffer_size_override_{0};
  uint16_t timeout_override_ms_{0};
  uint8_t tolerance_override_{0};
  uint16_t min_unknown_size_override_{0};
  uint32_t last_log_ms_{0};

  IRrecv *irrecv_{nullptr};
  decode_results results_{};
};

}  // namespace samsung_ac_sniffer
}  // namespace esphome
