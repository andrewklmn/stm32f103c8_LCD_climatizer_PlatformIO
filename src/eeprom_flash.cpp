#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "eeprom_flash.h"

FlashBuffer::FlashBuffer() {
    for (int i = 0; i < bufferSizeInPages; i++) {
      // check first busy marker word in page for understanding if this page is not used
      if(readEEPROMWord(i, 0) == 0xFFFFFFFF) {
        nextWritePageIndex = i; // initialize current value of next free memory page
        return;
      };
    }
}

FlashBuffer::~FlashBuffer() {
}

void FlashBuffer::readDataWordArray(uint32_t dataArray[], int dataArrayLength) {
  for (int i = 0; i < dataArrayLength; i++) {
    if (nextWritePageIndex == 0) {
      //read data from first page after first busy marker word
      dataArray[i] = readEEPROMWord(0, i + 1);       
    } else {
      //read data form the previous page after first busy marker word
      dataArray[i] = readEEPROMWord(nextWritePageIndex - 1, i + 1); 
    };
  }
}

void FlashBuffer::writeDataWordArray(uint32_t dataArray[], int dataArrayLength) {  

  // if last memory page is used, then reset all pages and write to first page
  if (nextWritePageIndex >= EEPROM_NUMBER_OF_PAGES) {
    eraseMemory();
    nextWritePageIndex = 0;
  };

  HAL_FLASH_Unlock();
  
  uint32_t address = bufferStartAddress + nextWritePageIndex * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE;
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, 0); // write first service marker word

  for (int i = 0; i < dataArrayLength; i++) {
    address  = bufferStartAddress 
    + nextWritePageIndex * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE  //current page address
    + (i + 1)* EEPROM_WORD_SIZE;  // current position address
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, dataArray[i]); // write data word

  };
  HAL_FLASH_Lock();
  nextWritePageIndex++;
}

void FlashBuffer::eraseMemory() {
  HAL_FLASH_Unlock();
  for (int i = 0; i < bufferSizeInPages; i++) {
    _FLASH_PageErase(bufferStartAddress + i * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE);
  };
  HAL_FLASH_Lock();
}

void FlashBuffer::_FLASH_PageErase(uint32_t PageAddress)
{
    /* Proceed to erase the page */
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    while (FLASH->SR & FLASH_SR_BSY);
    WRITE_REG(FLASH->AR, PageAddress);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
    while (FLASH->SR & FLASH_SR_BSY);
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
}

uint32_t FlashBuffer::readEEPROMWord(int page, int position) {
  uint32_t address  = bufferStartAddress 
    + page * EEPROM_WORDS_IN_PAGE * EEPROM_WORD_SIZE  // current page address
    + position * EEPROM_WORD_SIZE;  // current position address

  return *(__IO uint32_t*)address;
}
