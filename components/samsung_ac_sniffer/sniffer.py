import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID

CONF_RX_PIN = "rx_pin"
CONF_DUMP_RAW = "dump_raw"
CONF_DUMP_STATE = "dump_state"
CONF_DUMP_DESC = "dump_desc"
CONF_RATE_LIMIT_MS = "rate_limit_ms"
CONF_BUFFER_SIZE = "buffer_size"
CONF_TIMEOUT_MS = "timeout_ms"
CONF_TOLERANCE_PERCENT = "tolerance_percent"
CONF_MIN_UNKNOWN_SIZE = "min_unknown_size"

samsung_ac_sniffer_ns = cg.esphome_ns.namespace("samsung_ac_sniffer")
SamsungAcSniffer = samsung_ac_sniffer_ns.class_("SamsungAcSniffer", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SamsungAcSniffer),
        cv.Required(CONF_RX_PIN): cv.int_,
        cv.Optional(CONF_DUMP_RAW, default=True): cv.boolean,
        cv.Optional(CONF_DUMP_STATE, default=True): cv.boolean,
        cv.Optional(CONF_DUMP_DESC, default=True): cv.boolean,
        cv.Optional(CONF_RATE_LIMIT_MS, default=500): cv.positive_int,
        cv.Optional(CONF_BUFFER_SIZE): cv.positive_int,
        cv.Optional(CONF_TIMEOUT_MS): cv.int_range(min=1, max=255),
        cv.Optional(CONF_TOLERANCE_PERCENT): cv.int_range(min=1, max=50),
        cv.Optional(CONF_MIN_UNKNOWN_SIZE): cv.positive_int,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_rx_pin(config[CONF_RX_PIN]))
    cg.add(var.set_dump_raw(config[CONF_DUMP_RAW]))
    cg.add(var.set_dump_state(config[CONF_DUMP_STATE]))
    cg.add(var.set_dump_desc(config[CONF_DUMP_DESC]))
    cg.add(var.set_rate_limit_ms(config[CONF_RATE_LIMIT_MS]))
    if CONF_BUFFER_SIZE in config:
        cg.add(var.set_buffer_size(config[CONF_BUFFER_SIZE]))
    if CONF_TIMEOUT_MS in config:
        cg.add(var.set_timeout_ms(config[CONF_TIMEOUT_MS]))
    if CONF_TOLERANCE_PERCENT in config:
        cg.add(var.set_tolerance(config[CONF_TOLERANCE_PERCENT]))
    if CONF_MIN_UNKNOWN_SIZE in config:
        cg.add(var.set_min_unknown_size(config[CONF_MIN_UNKNOWN_SIZE]))
