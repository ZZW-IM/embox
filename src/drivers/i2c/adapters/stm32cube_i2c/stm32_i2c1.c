/**
 * @file
 *
 * @date    10.12.2018
 * @author  Alexander Kalmuk
 */
#include <string.h>
#include <assert.h>

#include <drivers/i2c/i2c.h>
#include <drivers/gpio.h>

#include <kernel/irq.h>

#include "stm32_i2c.h"

#include <config/board_config.h>

#include <bsp/stm32cube_hal.h>

#include <embox/unit.h>
#include <framework/mod/options.h>
#include <module/embox/driver/i2c/stm32cube_i2c.h>

EMBOX_UNIT_INIT(stm32_i2c1_init);

#define USE_I2C_IRQ \
	OPTION_MODULE_GET(embox__driver__i2c__stm32cube_i2c, BOOLEAN, use_i2c_irq)

static I2C_HandleTypeDef i2c1_handle;

static struct stm32_i2c stm32_i2c1_priv = {
	.i2c_handle = &i2c1_handle,
#if defined CONF_I2C1_REGION_BASE
	.i2c = (I2C_TypeDef *) CONF_I2C1_REGION_BASE,
#else
	.i2c = I2C1,
#endif	/* CONF_I2C1_REGION_BASE */
	.event_irq = CONF_I2C1_IRQ_EVENT,
	.error_irq = CONF_I2C1_IRQ_ERROR,
};

static struct i2c_adapter stm32_i2c1_adap = {
	.i2c_algo_data = &stm32_i2c1_priv,
	.i2c_algo = &stm32_i2c_algo,
};

static void stm32_i2c_gpio_init(I2C_HandleTypeDef *hi2c) {
	/* CONF_I2C1_CLK_ENABLE_SCL(); */
	gpio_setup_mode(CONF_I2C1_PIN_SCL_PORT, CONF_I2C1_PIN_SCL_NR,
		GPIO_MODE_ALT_SET(CONF_I2C1_PIN_SCL_AF) |
		GPIO_MODE_OUT_OPEN_DRAIN | GPIO_MODE_IN_PULL_UP);

	/* CONF_I2C1_CLK_ENABLE_SDA(); */
	gpio_setup_mode(CONF_I2C1_PIN_SDA_PORT, CONF_I2C1_PIN_SDA_NR,
		GPIO_MODE_ALT_SET(CONF_I2C1_PIN_SDA_AF) |
		GPIO_MODE_OUT_OPEN_DRAIN | GPIO_MODE_IN_PULL_UP);
}

static int stm32_i2c1_init(void) {
	if (0 > stm32_i2c_common_init(&stm32_i2c1_priv)) {
		return -1;
	}

	stm32_i2c_gpio_init(&i2c1_handle);

	CONF_I2C1_CLK_ENABLE_I2C();

	return i2c_bus_register(&stm32_i2c1_adap, 1, "i2c1");
}

#if USE_I2C_IRQ
extern irq_return_t i2c_ev_irq_handler(unsigned int irq_nr, void *data);
extern irq_return_t i2c_er_irq_handler(unsigned int irq_nr, void *data);

static_assert(CONF_I2C1_IRQ_EVENT == I2C1_EV_IRQn, "");
static_assert(CONF_I2C1_IRQ_ERROR == I2C1_EV_IRQn, "");

STATIC_IRQ_ATTACH(CONF_I2C1_IRQ_EVENT, i2c_ev_irq_handler, &i2c1_handle);
STATIC_IRQ_ATTACH(CONF_I2C1_IRQ_ERROR, i2c_er_irq_handler, &i2c1_handle);
#endif
