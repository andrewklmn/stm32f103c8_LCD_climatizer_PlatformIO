#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

/*
 *  EEPROM emulator for stm32f103
 * 
 *  Max size of stored array is 1023 bytes or 255 words!
 *
 *  USAGE:
 * 
 *  Put definition of constants in your main() file like this:
 *    #define EEPROM_START_ADDRESS   ((uint32_t)0x0800F800) // EEPROM emulation start address: after 62KByte of used Flash memory
 *    #define EEPROM_NUMBER_OF_PAGES  2                      // number of pages that is involved in buffer
 * 
 *  or like this:
 *    #define EEPROM_START_ADDRESS    ((uint32_t)0x08008400)  // EEPROM emulation start address: after 33KByte of used Flash memory
 *    #define EEPROM_NUMBER_OF_PAGES  3                      // number of pages that is involved in buffer
 *    #define SIZE_OF_STORED_ARRAY 255                       //  255 is max size due to memory page size
 * 
 *    uint32_t configArrayBuffer[SIZE_OF_STORED_ARRAY];      // config buffer array
 *    FlashBuffer memory( EEPROM_START_ADDRESS, EEPROM_NUMBER_OF_PAGES, SIZE_OF_STORED_ARRAY); // init memory buffer
 *
 * 
 *  Reading data from flash memory to buffer array:
 *     memory.readDataWordArray(dataBufferArray, SIZE_OF_STORED_ARRAY);
 *  Writing data from buffer array to flash memory:
 *     memory.writeDataWordArray(dataBufferArray, SIZE_OF_STORED_ARRAY);
 */

#define EEPROM_WORDS_IN_PAGE    256                     // number of words in memory page
#define EEPROM_WORD_SIZE        4                       // number of bytes in one word

class FlashBuffer
{
public:
  FlashBuffer(uint32_t startAddress, int numberOfPages, int dataArrayLength);
  virtual ~FlashBuffer();
  void readDataWordArray(uint32_t dataArray[]);
  void writeDataWordArray(uint32_t dataArray[]);
private:
  uint32_t bufferStartAddress;
  int bufferSizeInPages;
  int currentPageIndex;
  int dataLength;
  void eraseMemory();
  uint32_t readEEPROMWord(int page, int position);
};

#endif
