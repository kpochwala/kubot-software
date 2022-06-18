#include "eeprom/eeprom_helper.h"

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

static struct device* initialize_eeprom_device(void){
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

static void initialize_eeprom_start_module(struct eeprom_view *eeprom_ram_copy){
    eeprom_ram_copy->start_module.saved_state = 0;
    eeprom_ram_copy->start_module.dohyo_address = 0;
}

static void initialize_eeprom_tof(struct eeprom_view *eeprom_ram_copy){
    eeprom_ram_copy->tof.sensor_count = TOF_SENSOR_COUNT;
    for(int i = 0; i < eeprom_ram_copy->tof.sensor_count; i++){
        strcpy(eeprom_ram_copy->tof.sensor[i].sensor_name, "uninitialized");
        eeprom_ram_copy->tof.sensor[i].wants_initialization = true;
        eeprom_ram_copy->tof.sensor[i].enabled = true;
		eeprom_ram_copy->tof.sensor[i].xtalk_compensation_megacps = 0;
		eeprom_ram_copy->tof.sensor[i].offset_micrometer = 0;
    }
}

static void initialize_eeprom_line(struct eeprom_view *eeprom_ram_copy){
    eeprom_ram_copy->line.sensor_count = LINE_SENSOR_COUNT;
    for(int i = 0; i < eeprom_ram_copy->line.sensor_count; i++){
        strcpy(eeprom_ram_copy->line.sensor[i].sensor_name, "uninitialized");
        eeprom_ram_copy->line.sensor[i].wants_initialization = true;
        eeprom_ram_copy->line.sensor[i].enabled = true;

        eeprom_ram_copy->line.sensor[i].threshold = -10;
    }
}

static void initialize_eeprom_eeprom_configuration(struct eeprom_view *eeprom_ram_copy){
    eeprom_ram_copy->config.is_eeprom_initialized = true;
	eeprom_ram_copy->config.magic_number_of_initialization = 0xEFBEADDE;
}



void initialize_kubot_eeprom(){

	struct eeprom_view copy;

    initialize_eeprom_start_module(&copy);
    initialize_eeprom_tof(&copy);
    initialize_eeprom_line(&copy);
    initialize_eeprom_eeprom_configuration(&copy);

	int ret = 0;
	ret = eeprom_write(get_eeprom_device(), 0, &copy, sizeof(struct eeprom_view));

	volatile struct eeprom_view new_copy;
	read_eeprom_into(&new_copy);
	if(memcmp(&copy, &new_copy, sizeof(struct eeprom_view))){
		LOG_ERR("Writing to eeprom failed!");
	}
}

void read_eeprom_into(struct eeprom_view *ram_copy){

	struct device* dev = get_eeprom_device();
	if(dev == NULL){
		LOG_ERR("Could not find eeprom device!");
		return;
	}
	if(!device_is_ready(dev)){
		LOG_ERR("Device not ready!");
		return;
	}
	int ret = eeprom_read(get_eeprom_device(), 0, ram_copy, sizeof(struct eeprom_view));
	if(ret){
		LOG_ERR("EEPROM write failed as fuck");
	}
	if(ram_copy->config.magic_number_of_initialization != 0xEFBEADDE){
		LOG_ERR("Magic number mismatch! Reinitializing eeprom with default values!");
		initialize_kubot_eeprom();

	}
}

void write_eeprom(struct eeprom_view *ram_copy){
	struct device* dev = get_eeprom_device();
	if(dev == NULL){
		LOG_ERR("Could not find eeprom device!");
		return;
	}
	
	int ret = 0;
	ret = eeprom_write(get_eeprom_device(), 0, ram_copy, sizeof(struct eeprom_view));
	if(ret){
		LOG_ERR("Eeprom write failed as fuckkkkk");
	}

	struct eeprom_view new_copy;
	read_eeprom_into(&new_copy);
	k_sleep(K_MSEC(100));
	if(memcmp(&ram_copy, &new_copy, sizeof(struct eeprom_view))){
		LOG_ERR("Writing to eeprom failed!");
	}
}