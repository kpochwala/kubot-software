#include "eeprom_helper.h"

#include <zephyr/device.h>
#include <zephyr/drivers/eeprom.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(eeprom_helper);

static const struct device *get_eeprom_device(void)
{
	const struct device *dev = DEVICE_DT_GET(DT_ALIAS(eeprom_0));

	if (!device_is_ready(dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       dev->name);
		return NULL;
	}

	LOG_DBG("Found EEPROM device \"%s\"\n", dev->name);
	return dev;
}

struct device* initialize_eeprom(void){
    const struct device *eeprom = get_eeprom_device();
	size_t eeprom_size = 0;
    if(eeprom){
        eeprom_size = eeprom_get_size(eeprom);
        LOG_DBG("EEPROM initialized");
    }else{
        LOG_ERR("EEPROM initialization failed");
    }
	return eeprom;
}
