#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

//#define EEPROM_START_ADDRESS   ((uint32_t)0x08019000) // EEPROM emulation start address: after 100KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0801C000) // EEPROM emulation start address: after 112KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0800F800) // EEPROM emulation start address: after 62KByte of used Flash memory
//#define EEPROM_START_ADDRESS ((uint32_t)0x0800FC00)   // EEPROM emulation start address: after 63KByte of used Flash memory
#define EEPROM_START_ADDRESS    ((uint32_t)0x08008400)  // EEPROM emulation start address: after 33KByte of used Flash memory
#define EEPROM_WORDS_IN_PAGE    256                     // number of words in memory page
#define EEPROM_WORD_SIZE        4                       // number of bytes in one word
#define EEPROM_NUMBER_OF_PAGES  30                      // number of pages that is involved in buffer

class FlashBuffer
{
public:
  FlashBuffer();
  virtual ~FlashBuffer();
  void readDataWordArray(uint32_t dataArray[],int dataArrayLength);
  void writeDataWordArray(uint32_t dataArray[], int dataArrayLength);
  void eraseMemory();

private:
  uint32_t bufferStartAddress = EEPROM_START_ADDRESS;
  int bufferSizeInPages = EEPROM_NUMBER_OF_PAGES;
  int recordLengthInWords = 0;
  int nextWritePageIndex = 0;
  void _FLASH_PageErase(uint32_t);
  uint32_t readEEPROMWord(int page, int position);
};

#endif
