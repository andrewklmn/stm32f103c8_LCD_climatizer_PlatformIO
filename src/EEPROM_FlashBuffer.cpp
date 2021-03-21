#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "EEPROM_FlashBuffer.h"

EEPROM_FlashBuffer::EEPROM_FlashBuffer(uint32_t startAddress, int numberOfPages, int dataArrayLength) {
    bufferStartAddress = startAddress;
    bufferSizeInPages = numberOfPages;
    currentPageIndex = 0;
    dataLength = dataArrayLength;

    // try to find current used page
    for (int i = 0; i < bufferSizeInPages; i++) {
      // check first busy marker word in page for understanding if this page is not used
      if(readEEPROMWord(i, 0) == 0xFFFFFFFF) {
        // empty page is found... stop this search
        break;
      };
      currentPageIndex = i;  //this page contain stored array
    }
}

EEPROM_FlashBuffer::~EEPROM_FlashBuffer() {
}

void EEPROM_FlashBuffer::readDataWordArray(uint32_t dataArray[]) {
  for (int i = 0; i < dataLength; i++) {
    dataArray[i] = readEEPROMWord(currentPageIndex, i + 1);
  }
}

void EEPROM_FlashBuffer::writeDataWordArray(uint32_t dataArray[]) {
  if (currentPageIndex < bufferSizeInPages - 1 ) {
    // check if it is first write
    if (currentPageIndex == 0 && readEEPROMWord(0, 0) == 0xFFFFFFFF) {
      // will write to the first page 
    } else {
      // will write to next page
      currentPageIndex++;
    }
  } else {
    // last memory page is used!
    // erase all pages and start new page writing cycle from first page
    eraseMemory();
  };

  uint32_t address = bufferStartAddress + currentPageIndex * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE;

  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, 0); // write first service marker word
  for (int i = 0; i < dataLength; i++) {
    address += EEPROM_WORD_SIZE;                                      // add current position address
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, dataArray[i]); // write data word
  };
  HAL_FLASH_Lock();
}

void EEPROM_FlashBuffer::eraseMemory() {
  uint32_t PAGEError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = bufferStartAddress;
  EraseInitStruct.NbPages     = bufferSizeInPages;
  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
  HAL_FLASH_Lock();
  currentPageIndex = 0;
}

uint32_t EEPROM_FlashBuffer::readEEPROMWord(int page, int position) {
  uint32_t address  = bufferStartAddress 
                      + page * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE  // current page address
                      + position * EEPROM_WORD_SIZE;  // current position address
  return *(__IO uint32_t*)address;
}
