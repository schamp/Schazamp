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

#include <stdint.h>

class TwiMaster;

void i2c_eeprom_init(TwiMaster* twi);
bool i2c_eeprom_erase();
bool i2c_eeprom_write_buffer(uint32_t address, uint8_t* data, uint32_t length);
bool i2c_eeprom_write_buffer(uint8_t dev_id, uint16_t address, uint8_t* data, uint16_t length);
// data can be maximum of 128 bytes, according to the data sheet
// we may need to do some internal smarts here to determine page boundaries and break up write operations
bool i2c_eeprom_write_page(uint8_t dev_id, uint16_t eeaddress, uint8_t* data, uint8_t length );

uint8_t i2c_eeprom_read_byte(uint8_t dev_id, uint16_t eeaddress);
bool i2c_eeprom_read_buffer(uint32_t address, uint8_t* data, uint32_t length);
bool i2c_eeprom_read_buffer(uint8_t dev_id, uint16_t address, uint8_t *buffer, uint16_t length); 
