#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

//#define EEPROM_START_ADDRESS   ((uint32_t)0x08019000) // EEPROM emulation start address: after 100KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0801C000) // EEPROM emulation start address: after 112KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0800F800) // EEPROM emulation start address: after 63KByte of used Flash memory
#define EEPROM_START_ADDRESS ((uint32_t)0x0800FC00) // EEPROM emulation start address: after 62KByte of used Flash memory

class FlashBuffer
{
public:
  FlashBuffer();
  virtual ~FlashBuffer();
  uint32_t readWord();
  void writeWord(uint32_t);

private:
  uint32_t bufferStartAddress = EEPROM_START_ADDRESS;
  uint32_t currentWriteIndex = 0;
  uint32_t currentReadIndex = 0;
  int bufferSizeInWords = 400;

  void eraseMemory();    
  void _FLASH_PageErase(uint32_t);
  uint32_t readEEPROMWord(uint32_t address);
  HAL_StatusTypeDef writeEEPROMWord(uint32_t address, uint32_t data);
};

#endif
