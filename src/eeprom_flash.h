#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include <stdint.h>

//#define EEPROM_START_ADDRESS   ((uint32_t)0x08019000) // EEPROM emulation start address: after 100KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0801C000) // EEPROM emulation start address: after 112KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0800F800) // EEPROM emulation start address: after 63KByte of used Flash memory
#define EEPROM_START_ADDRESS   ((uint32_t)0x0800FC00) // EEPROM emulation start address: after 62KByte of used Flash memory


void enableEEPROMWriting(); // Unlock and keep PER cleared
void disableEEPROMWriting(); // Lock

// Write functions
HAL_StatusTypeDef writeEEPROMWord(uint32_t address, uint32_t data);

// Read functions
uint32_t readEEPROMWord(uint32_t address);

#endif
