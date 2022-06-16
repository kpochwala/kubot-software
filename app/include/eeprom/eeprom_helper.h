#pragma once
#include "eeprom/eeprom_structs.h"

void initialize_kubot_eeprom();
void read_eeprom_into(struct eeprom_view *ram_copy);
void write_eeprom(struct eeprom_view *ram_copy);