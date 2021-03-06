/*
 *  DRAGINO V2 board support, based on Atheros AP121 board support
 *  
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2012-2013 Elektra Wagenrad <elektra@villagetelco.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
// Added for SPI bitbanging support
#include <linux/spi/spi_gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/leds.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/gpio.h>



#define DRAGINO2_GPIO_LED_WLAN		0
#define DRAGINO2_GPIO_LED_LAN		13
#define DRAGINO2_GPIO_LED_WAN		17


/* The following GPIO is actually named "Router" on the board. 
 * However, since the "Router" feature is not supported as of yet
 * we use it to display USB activity. 
 */

#define DRAGINO2_GPIO_LED_USB		28
#define DRAGINO2_GPIO_BTN_JUMPSTART	11
#define DRAGINO2_GPIO_BTN_RESET		12

#define DRAGINO2_KEYS_POLL_INTERVAL	20	/* msecs */
#define DRAGINO2_KEYS_DEBOUNCE_INTERVAL	(3 * DRAGINO2_KEYS_POLL_INTERVAL)

#define DRAGINO2_MAC0_OFFSET		0x0000
#define DRAGINO2_MAC1_OFFSET		0x0006
#define DRAGINO2_CALDATA_OFFSET		0x1000
#define DRAGINO2_WMAC_MAC_OFFSET		0x1002



static struct gpio_led dragino2_leds_gpio[] __initdata = {
	{
		.name		= "dragino2:red:lan",
		.gpio		= DRAGINO2_GPIO_LED_LAN,
		.active_low	= 0,
	},
	{
		.name		= "dragino2:red:wlan",
		.gpio		= DRAGINO2_GPIO_LED_WLAN,
		.active_low	= 0,
	},
		{
		.name		= "dragino2:red:wan",
		.gpio		= DRAGINO2_GPIO_LED_WAN,
		.active_low	= 0,
	},
	{
		.name		= "dragino2:red:usb",
		.gpio		= DRAGINO2_GPIO_LED_USB,
		.active_low	= 0,
	},
	
	
};

static struct gpio_keys_button dragino2_gpio_keys[] __initdata = {
	{
		.desc		= "jumpstart button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DRAGINO2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DRAGINO2_GPIO_BTN_JUMPSTART,
		.active_low	= 1,
	},
	{
		.desc		= "reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DRAGINO2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DRAGINO2_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

/* Support for a second SPI Master with ID-number 1
 * by bitbanging GPIOs
 * 
 * GPIO resources used by for the Si3217x-based FXS SPI slave module:
 * 
 * CS (Chip select) for FXS board, SPI slave device #0, selected by GPIO_24 @Router_header_PIN12
 * FXS_RESET = GPIO 23 @Router_header_PIN9
 * FXS_INT = GPIO27 = @JTAG_header_PIN2
 * SCLK = GPIO_15 = @JTAG_header_PIN9
 * MOSI = GPIO_16 = @JTAG_header_PIN10
 * MISO = GPIO_26 = @JTAG_header_PIN1
*/


static struct spi_gpio_platform_data dragino2_20_spi_gpio_data = {
.sck = 15,
.mosi = 16,
.miso = 26,
.num_chipselect = 1
};

static struct platform_device dragino2_spi_gpio_device = {
    .name		= "spi_gpio",
    .id			= 1,
    .dev.platform_data = &dragino2_20_spi_gpio_data,
};



// WiP: Add SPI client support for the FXS module

/*static struct si3217x_platform_data si3217x_info [] = {
	{
	 .reset = 23,
	},
};*/ 

static struct spi_board_info si3217x_spi_info[] = {
	{
		.bus_num	= 1,
		.max_speed_hz	= 10000000,
		.modalias	= "si3217x",
		.chip_select	= 0,
		.controller_data        = (void *) 24,
		.mode 		= 3,
		.irq		= 27,
	}
};



static void __init dragino2_common_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u32 val;

	ath79_register_m25p80(NULL);
	ath79_register_wmac(art + DRAGINO2_CALDATA_OFFSET,
			    art + DRAGINO2_WMAC_MAC_OFFSET);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + DRAGINO2_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + DRAGINO2_MAC1_OFFSET, 0);
	
	ath79_register_mdio(0, 0x0);
	
	//Enable GPIO15 and GPIO16
	ath79_gpio_function_disable(
				   AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				   AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN    
                                  );	
 
	/* LAN ports */
	ath79_register_eth(1);

	/* WAN port */
	ath79_register_eth(0);
	
	
	/* Enable GPIO26 and GPIO27 */
	/* Read reset register */
	val = ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	
	val |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	
	/* Write reset register */
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP, val);
}



static void __init dragino2_setup(void)
{
	printk(KERN_INFO "In dragino2_setup()\n");	

	dragino2_common_setup();
		
	ath79_register_leds_gpio(-1, ARRAY_SIZE(dragino2_leds_gpio),
				 dragino2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DRAGINO2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dragino2_gpio_keys),
					dragino2_gpio_keys);
	ath79_register_usb();
	
	spi_register_board_info(si3217x_spi_info,ARRAY_SIZE(si3217x_spi_info));
	
	platform_device_register(&dragino2_spi_gpio_device);
}


MIPS_MACHINE(ATH79_MACH_DRAGINO2, "DRAGINO2", "Dragino Dragino v2",
	     dragino2_setup);

