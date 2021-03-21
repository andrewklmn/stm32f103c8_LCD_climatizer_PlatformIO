#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "EEPROM_FlashBuffer.h"

EEPROM_FlashBuffer::EEPROM_FlashBuffer(uint32_t startAddress, int numberOfPages, int givenDataArrayLength) {
    bufferStartAddress = startAddress;
    bufferSizeInPages = numberOfPages;
    dataArrayLength = givenDataArrayLength;
    currentPageIndex = 0;
    currentPositionIndex = 0;
    nextPositionStep = givenDataArrayLength + 1;

    // try to find current used page
    for (int i = 0; i < bufferSizeInPages; i++) {
      // check first busy marker word in page for understanding if this page is not used
      if(readEEPROMWord(i, 0) == 0xFFFFFFFF) {
        // empty page is found... So lets try to find nearest empty position on previous page
        for (int j = 0; j < EEPROM_WORDS_IN_PAGE; j += nextPositionStep ) {
          if (readEEPROMWord(currentPageIndex, j) == 0xFFFFFFFF) {
            // empty position found
            break;
          }
          currentPositionIndex = j;
        }  
        break;
      };
      currentPageIndex = i;  //this page contain stored array
    }
}

EEPROM_FlashBuffer::~EEPROM_FlashBuffer() {
}

void EEPROM_FlashBuffer::readDataWordArray(uint32_t dataArray[]) {
  int dataAddress = currentPositionIndex + 1;
  for (int i = 0; i < dataArrayLength; i++) {
    dataArray[i] = readEEPROMWord(currentPageIndex, dataAddress + i);
  }
}

void EEPROM_FlashBuffer::writeDataWordArray(uint32_t dataArray[]) {
  
  uint32_t address = bufferStartAddress;

  // check if it is the first write session. So it will write array from first position
  if (currentPageIndex == 0 && currentPositionIndex == 0 && readEEPROMWord(0, 0) == 0xFFFFFFFF) {
    // write as first time

  } else if (currentPageIndex <= (bufferSizeInPages - 1)  
              && readEEPROMWord(currentPageIndex, currentPositionIndex) == 0) {

    // check if the next position will fit in current page 
    if (currentPositionIndex + nextPositionStep
            <= EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE - nextPositionStep) {
      // This data array fits in the free space on current page

      // Write data array to the next position
      currentPositionIndex += nextPositionStep;

    } else {
      // This array doesn't fit into this page, we need the next page to write it
      
      // Check if we have one more free page
      if (currentPageIndex < bufferSizeInPages - 1) {
        // it will write to the next page in first position
        currentPageIndex++;
        currentPositionIndex = 0;
      } else {
        //format all memory
        eraseMemory();
        // and write as first time
      }
    }
  };

  address += currentPageIndex * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE + currentPositionIndex;

  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, 0); // write page service marker word
  // write array data
  for (int i = 0; i < dataArrayLength; i++) {
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
  currentPositionIndex = 0;
}

uint32_t EEPROM_FlashBuffer::readEEPROMWord(int page, int position) {
  uint32_t address  = bufferStartAddress 
                      + page * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE  // current page address
                      + position * EEPROM_WORD_SIZE;  // current position address
  return *(__IO uint32_t*)address;
}
