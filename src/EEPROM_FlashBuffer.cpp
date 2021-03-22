#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "EEPROM_FlashBuffer.h"

EEPROM_FlashBuffer::EEPROM_FlashBuffer(uint32_t startAddress, int numberOfPages, int arrayLength)
{
  bufferStartAddress = startAddress;
  numberOfBufferPages = numberOfPages;
  bufferLengthInWords = numberOfPages * EEPROM_WORDS_IN_PAGE;
  dataArrayLengthInWords = arrayLength;
  recordLengthInWords = arrayLength + 1;
  nextWriteWordIndex = 0;

  // find first empty word index
  for (int i = 0; i < bufferLengthInWords; i += recordLengthInWords)
  {
    if (readEEPROMWord(i) == 0xFFFFFFFF)
    { // buffer is empty
      nextWriteWordIndex = i;
      break;
    };
  }
}

EEPROM_FlashBuffer::~EEPROM_FlashBuffer()
{
}

void EEPROM_FlashBuffer::readArray(uint32_t dataArray[])
{
  if (nextWriteWordIndex == 0)
  { // fill array with zero because it was not initialized
    for (int i = 0; i < dataArrayLengthInWords; i++)
    {
      dataArray[i] = 0;
    }
  }
  else
  { // fill array with last stored data
    for (int i = 0; i < dataArrayLengthInWords; i++)
    {
      dataArray[i] = readEEPROMWord(nextWriteWordIndex - recordLengthInWords + 1 + i);
    }
  }
}

void EEPROM_FlashBuffer::writeArray(uint32_t dataArray[])
{
  bool erasingIsNeeded = false;

  if (nextWriteWordIndex > bufferLengthInWords - recordLengthInWords)
  {
    nextWriteWordIndex = 0;
    erasingIsNeeded = true;
  }

  HAL_FLASH_Unlock();
  if (erasingIsNeeded)
  {
    eraseMemory();
  }
  uint32_t address = bufferStartAddress + nextWriteWordIndex * EEPROM_WORD_SIZE;
  // write service marker word before array data
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, ((uint64_t)nextWriteWordIndex));
  // write array data after service marker word
  for (int i = 0; i < dataArrayLengthInWords; i++)
  {
    address += EEPROM_WORD_SIZE;
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, (uint64_t)dataArray[i]);
  };
  HAL_FLASH_Lock();

  nextWriteWordIndex += recordLengthInWords;
}

void EEPROM_FlashBuffer::eraseMemory()
{
  FLASH_EraseInitTypeDef f;
  f.TypeErase = FLASH_TYPEERASE_PAGES;
  f.PageAddress = bufferStartAddress;
  f.NbPages = numberOfBufferPages;
  uint32_t PageError = 0;
  HAL_FLASHEx_Erase(&f, &PageError);
}

void EEPROM_FlashBuffer::clearBuffer()
{
  HAL_FLASH_Unlock();
  eraseMemory();
  HAL_FLASH_Lock();
  
}

uint32_t EEPROM_FlashBuffer::readEEPROMWord(int wordIndex)
{
  uint32_t address = bufferStartAddress + wordIndex * EEPROM_WORD_SIZE; // add word address
  return *(__IO uint32_t *)address;
}
