import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import climate
from esphome.const import CONF_ID

CONF_IR_PIN = "ir_pin"
CONF_SUPPORTS_SWING_HORIZONTAL = "supports_swing_horizontal"
CONF_SUPPORTS_SWING_VERTICAL = "supports_swing_vertical"

samsung_ac_ir_ns = cg.esphome_ns.namespace("samsung_ac_ir")
SamsungAcIrClimate = samsung_ac_ir_ns.class_(
    "SamsungAcIrClimate", climate.Climate, cg.Component
)

# ESPHome 2025.11+ removed the *_SCHEMA constants. Prefer the new helper and
# keep a fallback for older ESPHome versions.
try:
    BASE_SCHEMA = climate.climate_schema(SamsungAcIrClimate)
except AttributeError:
    BASE_SCHEMA = climate.CLIMATE_SCHEMA

CONFIG_SCHEMA = BASE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SamsungAcIrClimate),
        cv.Required(CONF_IR_PIN): cv.int_,
        cv.Optional(CONF_SUPPORTS_SWING_HORIZONTAL, default=False): cv.boolean,
        cv.Optional(CONF_SUPPORTS_SWING_VERTICAL, default=False): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    cg.add(var.set_ir_pin(config[CONF_IR_PIN]))
    cg.add(var.set_supports_swing_horizontal(config[CONF_SUPPORTS_SWING_HORIZONTAL]))
    cg.add(var.set_supports_swing_vertical(config[CONF_SUPPORTS_SWING_VERTICAL]))
