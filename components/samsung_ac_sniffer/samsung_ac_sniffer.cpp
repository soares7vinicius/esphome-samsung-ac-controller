#include "samsung_ac_sniffer.h"

namespace esphome {
namespace samsung_ac_sniffer {

const char *const SamsungAcSniffer::TAG = "samsung_sniffer";

void SamsungAcSniffer::setup() {
  // Use IRrecv with a modest buffer; choose defaults compatible with IRremoteESP8266 2.x,
  // but allow user overrides via YAML.
#if defined(kCaptureBufferSize)
  uint16_t bufsize = kCaptureBufferSize;
#elif defined(kRawBuf)
  uint16_t bufsize = kRawBuf;
#else
  uint16_t bufsize = 1024;
#endif
  if (this->buffer_size_override_ > 0)
    bufsize = this->buffer_size_override_;

#if defined(kTimeoutDefault)
  uint16_t timeout = kTimeoutDefault;
#else
  uint16_t timeout = 50;
#endif
  if (this->timeout_override_ms_ > 0)
    timeout = this->timeout_override_ms_;
  if (timeout > 255)
    timeout = 255;  // IRrecv expects uint8_t; clamp for safety.

  this->irrecv_ = new IRrecv(this->rx_pin_, bufsize, static_cast<uint8_t>(timeout), true);
  if (this->tolerance_override_ > 0)
    this->irrecv_->setTolerance(this->tolerance_override_);
  if (this->min_unknown_size_override_ > 0)
    this->irrecv_->setUnknownThreshold(this->min_unknown_size_override_);
  this->irrecv_->enableIRIn();

  ESP_LOGI(TAG, "IR sniffer listening on GPIO%u", this->rx_pin_);
}

void SamsungAcSniffer::loop() {
  if (this->irrecv_ == nullptr)
    return;

  if (!this->irrecv_->decode(&this->results_))
    return;

  const uint32_t now = millis();
  if (now - this->last_log_ms_ >= this->rate_limit_ms_) {
    this->log_result_();
    this->last_log_ms_ = now;
  }

  this->irrecv_->resume();
}

void SamsungAcSniffer::log_result_() {
  const auto proto = typeToString(this->results_.decode_type, this->results_.repeat);
  ESP_LOGI(TAG, "Protocol  : %s", proto.c_str());

  const uint16_t bits = this->results_.bits;
  size_t nbytes = static_cast<size_t>((bits + 7) / 8);
  if (nbytes > kStateSizeMax)
    nbytes = kStateSizeMax;

  if (nbytes > 0 && nbytes <= kStateSizeMax) {
    const auto code_hex = this->format_state_hex_(nbytes);
    if (!code_hex.empty()) {
      ESP_LOGI(TAG, "Code      : 0x%s (%u Bits)", code_hex.c_str(), bits);
    }

    if (this->dump_state_) {
      this->log_state_array_(nbytes);
    }
  }

  if (this->dump_desc_) {
    const auto desc = IRAcUtils::resultAcToString(&this->results_);
    if (desc.length() > 0) {
      ESP_LOGI(TAG, "Mesg Desc.: %s", desc.c_str());
    }
  }

  if (this->dump_raw_) {
    const auto raw = resultToSourceCode(&this->results_);
    if (raw.length() > 0) {
      ESP_LOGI(TAG, "%s", raw.c_str());
    }
  }
}

std::string SamsungAcSniffer::format_state_hex_(size_t nbytes) const {
  if (nbytes == 0 || nbytes > kStateSizeMax)
    return {};

  std::string hex;
  hex.reserve(nbytes * 2);
  for (size_t i = 0; i < nbytes; i++) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02X", this->results_.state[i]);
    hex += buf;
  }
  return hex;
}

void SamsungAcSniffer::log_state_array_(size_t nbytes) const {
  if (nbytes == 0 || nbytes > kStateSizeMax)
    return;

  // Format like IRrecvDumpV3: uint8_t state[21] = {0x..};
  std::string out = "uint8_t state[";
  out += std::to_string(nbytes);
  out += "] = {";

  for (size_t i = 0; i < nbytes; i++) {
    if (i > 0)
      out += ", ";
    char buf[5];
    snprintf(buf, sizeof(buf), "0x%02X", this->results_.state[i]);
    out += buf;
  }

  out += "};";
  ESP_LOGI(TAG, "%s", out.c_str());
}

}  // namespace samsung_ac_sniffer
}  // namespace esphome
