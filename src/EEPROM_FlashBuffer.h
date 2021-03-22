#ifndef __EEPROM_FLASH_BUFFER_H
#define __EEPROM_FLASH_BUFFER_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

/*
 *  EEPROM emulator for stm32f103
 * 
 *  USAGE:
 * 
 *  Put definition of constants in your main() file like this:
 *    #define EEPROM_START_ADDRESS   ((uint32_t)0x0800F800) // EEPROM emulation start address: after 62KByte
 *    #define EEPROM_NUMBER_OF_PAGES  2                      // number of pages that is involved in buffer
 *    #define SIZE_OF_STORED_ARRAY 100
 * 
 *  or like this:
 *    #define EEPROM_START_ADDRESS    ((uint32_t)0x08008400)  // EEPROM emulation start address: after 33KByte
 *    #define EEPROM_NUMBER_OF_PAGES  5                       // number of pages that are involved in buffer
 *    #define SIZE_OF_STORED_ARRAY 100
 * 
 *  
 *    EEPROM_FlashBuffer memory( EEPROM_START_ADDRESS, EEPROM_NUMBER_OF_PAGES, SIZE_OF_STORED_ARRAY); // init memory buffer
 *    uint32_t dataBufferArray[SIZE_OF_STORED_ARRAY];         // buffer for reading and writing array to EEPROM
 *
 * 
 *  Reading data from flash memory to buffer array:
 *     memory.readArray(dataBufferArray);
 * 
 *  Writing data from buffer array to flash memory:
 *     memory.writeArray(dataBufferArray);
 */

#define EEPROM_WORDS_IN_PAGE 256 // number of words in memory page
#define EEPROM_WORD_SIZE 4       // number of bytes in one word

class EEPROM_FlashBuffer
{
public:
  EEPROM_FlashBuffer(uint32_t startAddress, int numberOfPages, int givenDataArrayLength);
  virtual ~EEPROM_FlashBuffer();
  void readArray(uint32_t dataArray[]);
  void writeArray(uint32_t dataArray[]);
  void clearBuffer();

private:
  uint32_t bufferStartAddress;
  int numberOfBufferPages;
  int bufferLengthInWords;
  int dataArrayLengthInWords;
  int recordLengthInWords;
  int nextWriteWordIndex;
  void eraseMemory();
  uint32_t readEEPROMWord(int wordIndex);
};

#endif
