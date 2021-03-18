#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "eeprom_flash.h"

FlashBuffer::FlashBuffer() {
    for (int i = 0; i < bufferSizeInWords; i++) {
        if(readEEPROMWord(i) == 0xFFFFFFFF) {
            currentWriteIndex = i;
            if (i > 0) currentReadIndex = i - 1; 
            break;
        } else {
            currentWriteIndex = i + 1;
            currentReadIndex = i;
        };
    }
}

FlashBuffer::~FlashBuffer() {
}

uint32_t FlashBuffer::readWord() {
    return readEEPROMWord(currentReadIndex);
}

void FlashBuffer::writeWord(uint32_t word) {   
    if (currentWriteIndex == (uint32_t)bufferSizeInWords) {
        eraseMemory();
        currentWriteIndex = 0;
    };
    writeEEPROMWord(currentWriteIndex, word);
    currentReadIndex = currentWriteIndex;
    currentWriteIndex++;
}

void FlashBuffer::eraseMemory() {
  HAL_FLASH_Unlock();
  _FLASH_PageErase(bufferStartAddress);
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


HAL_StatusTypeDef FlashBuffer::writeEEPROMWord(uint32_t address, uint32_t data) {
  HAL_StatusTypeDef status;
  address = address + bufferStartAddress;
  HAL_FLASH_Unlock();
  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
  HAL_FLASH_Lock();
  return status;
}

uint32_t FlashBuffer::readEEPROMWord(uint32_t address) {
    address = address + bufferStartAddress;
    return *(__IO uint32_t*)address;
}
