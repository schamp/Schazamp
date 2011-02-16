/* 24AA1025 EEPROM Library
 * Copyright (C) 2010 by Andrew Schamp 
 *
 * This packages was produced under no affiliation with Microchip, the maker of this device
 *
 * This file is part of the 24AA1025 EEPROM Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the 24AA1025 EEPROM Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */
#include "eeprom_24aa1025.h"

#include <TwiMaster.h>

//TwiMaster my_twi;

#define EEPROM_ID_PREFIX B1010

TwiMaster* my_twi = NULL;

// reverse dev offset 
// 000 - first 'device' on 1st chip
// 100 - second 'device' on 1st chip
// 001 - first 'device' on 2nd chip
// 101 - second 'device' on 2nd chip
// etc.
uint8_t DEV_REVERSE_LOOKUP[] = { 
  B000, B100, B001, B101, B010, B110, B011, B111
};

// how large is the address space of a single EEPROM "device"
// (each chip has two "devices", selectable by the "block select" address bit)
#define DEVICE_SIZE 0x10000
#ifndef EEPROM_CHIPS
#define EEPROM_CHIPS 1
#endif
// two devices per chip
#define DEVICES      (EEPROM_CHIPS*2)
#define MAX_ADDR     (DEVICES*DEVICE_SIZE)

#define PAGE_SIZE 0x80

void i2c_eeprom_init(TwiMaster* twi) {
  my_twi = twi;
}

bool i2c_eeprom_erase() {
  // initialize all bytes to 0
  uint8_t data[128] = { 0 };
  uint32_t addr = 0x0;
  // erase each page 
//  Serial.print("Erasing EEPROM");
  while (addr < MAX_ADDR) {
    i2c_eeprom_write_buffer(addr, data, sizeof(data));
    addr += sizeof(data); 
//    Serial.print(".");
  }
//  Serial.println("done.");
}

bool i2c_eeprom_write_buffer(uint32_t address, uint8_t* data, uint32_t length) {
  if (address > MAX_ADDR) {
//    Serial.print("Address 0x");
//    Serial.print(address, HEX);
//    Serial.print(" out of bounds: 0x0 - 0x");
//    Serial.println(MAX_ADDR, HEX);
    return false;
  }
  if (address + length > MAX_ADDR) {
//    Serial.print("Buffer starting at 0x");
//    Serial.print(address, HEX);
//    Serial.print(" with length ");
//    Serial.print(length, DEC);
//    Serial.print(" overruns end of address space (0x");
//    Serial.print(MAX_ADDR, HEX);
//    Serial.println(")");
    return false;
  }
  uint32_t start_byte        = address;
  uint32_t end_byte          = address + length;
  uint32_t curr_device_start = (address / DEVICE_SIZE) * DEVICE_SIZE;
  uint32_t next_device_start = curr_device_start + DEVICE_SIZE;

  bool success = true;
  while (end_byte > next_device_start && success) {
    uint8_t dev_offset = start_byte / DEVICE_SIZE;
    uint8_t dev_id     = (EEPROM_ID_PREFIX << 3) | DEV_REVERSE_LOOKUP[dev_offset];
    success = i2c_eeprom_write_buffer(dev_id, (uint16_t)(start_byte % DEVICE_SIZE), (uint8_t*)&(data[start_byte - address]), next_device_start - start_byte);

    curr_device_start = next_device_start;
    next_device_start = curr_device_start + DEVICE_SIZE;
    start_byte        = curr_device_start;
  }
  if (!success) {
    return false;
  }
  /*
  Serial.print("write_buffer writing ");
   Serial.print(length, DEC);
   Serial.print(" bytes to address ");
   Serial.print(address, HEX);
   Serial.print(" on device ");
   Serial.println(dev_id, HEX);
   */
  uint8_t dev_offset = start_byte / DEVICE_SIZE;
  uint8_t dev_id     = (EEPROM_ID_PREFIX << 3) | DEV_REVERSE_LOOKUP[dev_offset];
  return i2c_eeprom_write_buffer(dev_id, (uint16_t)(start_byte % DEVICE_SIZE), (uint8_t*)&(data[start_byte - address]), end_byte - start_byte);
}


bool i2c_eeprom_write_buffer(uint8_t dev_id, uint16_t address, uint8_t* data, uint16_t length) {
  uint16_t start_byte = address;
  uint16_t end_byte   = address + length;

  uint16_t curr_page_start = (address / PAGE_SIZE) * PAGE_SIZE;
  uint16_t next_page_start = curr_page_start + PAGE_SIZE;

  bool success = true;
  while (end_byte > next_page_start && success) {
    /*
    Serial.print("device-specific write_buffer writing ");
     Serial.print(next_page_start - start_byte, DEC);
     Serial.print(" bytes from data[");
     Serial.print(start_byte - address, DEC);
     Serial.print("] (");
     Serial.print((uint16_t)(&data[start_byte - address]), HEX);
     Serial.print(") to address ");
     Serial.println(start_byte, HEX);
     */
    success = i2c_eeprom_write_page(dev_id, start_byte, &(data[start_byte - address]), next_page_start - start_byte);
    curr_page_start = next_page_start;
    next_page_start = curr_page_start + PAGE_SIZE;
    start_byte = curr_page_start;
  }
  if (!success) {
    return false;
  }
  /*  
   Serial.print("device-specific write_buffer writing final ");
   Serial.print(end_byte - start_byte, DEC);
   Serial.print(" bytes from data[");
   Serial.print(start_byte - address, DEC);
   Serial.print("] (");
   Serial.print((uint16_t)(&data[start_byte - address]), HEX);
   Serial.print(") to address ");
   Serial.println(start_byte, HEX);
   */
  return i2c_eeprom_write_page(dev_id, start_byte, &(data[start_byte - address]), end_byte - start_byte);
}

// data can be maximum of 128 bytes, according to the data sheet
// we may need to do some internal smarts here to determine page boundaries and break up write operations
bool i2c_eeprom_write_page(uint8_t dev_id, uint16_t eeaddress, uint8_t* data, uint8_t length ) {
  /*
  Serial.print("write page: dev ");
   Serial.print(dev_id, HEX);
   Serial.print(" address: ");
   Serial.print(eeaddress, HEX);
   Serial.print(" data_ptr: ");
   Serial.print((uint16_t)data, HEX);
   Serial.print(" length: ");
   Serial.println(length, DEC);
   */
  if (my_twi->start(dev_id, I2C_WRITE)) {
    my_twi->write((uint8_t)((eeaddress >> 8) &0xFF));
    my_twi->write((uint8_t)(eeaddress & 0xFF));
    for (uint8_t c = 0; c < length; c++) {
      my_twi->write(data[c]);
    }
    my_twi->stop();
    // 24AA1025 will not acknowledge start conditions until the write cycle is complete
    while(!my_twi->start(dev_id, I2C_WRITE)) {
    };
    my_twi->stop();

    return true;
  } 
  else {
//    Serial.println("nack for dev_id / write");
    return false;
  }
}

uint8_t i2c_eeprom_read_byte(uint8_t dev_id, uint16_t eeaddress ) {
  uint8_t b = 0;
  if (my_twi->start(dev_id, I2C_WRITE)) {
    my_twi->write((uint8_t)((eeaddress >> 8) &0xFF));
    my_twi->write((uint8_t)(eeaddress & 0xFF));
    my_twi->start(dev_id, I2C_READ);
    b = my_twi->read(true);
    my_twi->stop();
  } 
  else {
//    Serial.println("nack for dev_id / write");
  } 
  return b;
}

bool i2c_eeprom_read_buffer(uint32_t address, uint8_t* data, uint32_t length) {
  if (address > MAX_ADDR) {
//    Serial.print("Address ");
//    Serial.print(address, HEX);
//    Serial.print(" out of bounds: 0x0 - 0x");
//    Serial.println(MAX_ADDR, HEX);
    return false;
  }
  if (address + length > MAX_ADDR) {
//    Serial.print("Buffer starting at 0x");
//    Serial.print(address, HEX);
//    Serial.print(" with length ");
//    Serial.print(length, DEC);
//    Serial.print(" overruns end of address space (0x");
//    Serial.print(MAX_ADDR, HEX);
//    Serial.println(")");
    return false;
  }

  uint32_t start_byte        = address;
  uint32_t end_byte          = address + length;
  uint32_t curr_device_start = (address / DEVICE_SIZE) * DEVICE_SIZE;
  uint32_t next_device_start = curr_device_start + DEVICE_SIZE;

  bool success = true;
  while (end_byte > next_device_start && success) {
    uint8_t dev_offset = start_byte / DEVICE_SIZE;
    uint8_t dev_id     = (EEPROM_ID_PREFIX << 3) | DEV_REVERSE_LOOKUP[dev_offset];
    success = i2c_eeprom_read_buffer(dev_id, (uint16_t)(start_byte % DEVICE_SIZE), (uint8_t*)&(data[start_byte - address]), next_device_start - start_byte);

    curr_device_start = next_device_start;
    next_device_start = curr_device_start + DEVICE_SIZE;
    start_byte        = curr_device_start;
  }
  if (!success) {
    return false;
  }
  /*
  Serial.print("read_buffer reading ");
   Serial.print(length, DEC);
   Serial.print(" bytes from address ");
   Serial.print(address, HEX);
   Serial.print(" on device ");
   Serial.println(dev_id, HEX);
   */
  uint8_t dev_offset = start_byte / DEVICE_SIZE;
  uint8_t dev_id     = (EEPROM_ID_PREFIX << 3) | DEV_REVERSE_LOOKUP[dev_offset];
  return i2c_eeprom_read_buffer(dev_id, (uint16_t)(start_byte % DEVICE_SIZE), (uint8_t*)&(data[start_byte - address]), (uint16_t)(end_byte - start_byte));
}

bool i2c_eeprom_read_buffer(uint8_t dev_id, uint16_t address, uint8_t *buffer, uint16_t length ) {
  uint8_t i = 0;
  if (my_twi->start(dev_id, I2C_WRITE)) {
    my_twi->write((uint8_t)((address >> 8) &0xFF));
    my_twi->write((uint8_t)(address & 0xFF));
    my_twi->start(dev_id, I2C_READ);
    while (i < length-1) {
      buffer[i++] = my_twi->read(false);
    }
    buffer[i] = my_twi->read(true);
    my_twi->stop();
    return true;
  } 
  else {
//    Serial.println("nack for dev_id / write");
    return false;
  } 
}

