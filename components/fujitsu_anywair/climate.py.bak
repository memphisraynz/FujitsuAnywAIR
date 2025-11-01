import esphome.config_validation as cv
import esphome.codegen as cg

from esphome.components import (
    climate,
    uart,
    sensor
)
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_SENSOR,
    CONF_UART_ID,
    CONF_SUPPORTS_COOL,
    CONF_SUPPORTS_HEAT,
)
from esphome.core import coroutine

CODEOWNERS = ["@memphisraynz"]
DEPENDENCIES = ["climate", "sensor"]
AUTO_LOAD = ["uart"]

fujitsu_ac_ns = cg.esphome_ns.namespace("fujitsu_anywair")
FujitsuAnywAIRClimate = fujitsu_ac_ns.class_("FujitsuAnywAIRClimate", climate.Climate, cg.Component)

CONFIG_SCHEMA = cv.All(
    climate.climate_schema(FujitsuAnywAIRClimate)
    .extend(
        {
            cv.GenerateID(): cv.declare_id(FujitsuAnywAIRClimate),
            cv.Optional(CONF_SUPPORTS_COOL, default=True): cv.boolean,
            cv.Optional(CONF_SUPPORTS_HEAT, default=True): cv.boolean,
            cv.Optional(CONF_SENSOR): cv.use_id(sensor.Sensor),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_SENSOR in config:
        sens = await cg.get_variable(config[CONF_SENSOR])
        cg.add(var.set_sensor(sens))

    cg.add(var.set_supports_cool(config[CONF_SUPPORTS_COOL]))
    cg.add(var.set_supports_heat(config[CONF_SUPPORTS_HEAT]))

    uart_ = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_))
