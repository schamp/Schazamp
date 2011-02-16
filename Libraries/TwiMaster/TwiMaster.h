/* Arduino TwiMaster Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino TwiMaster Library
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
 * along with the Arduino TwiMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef TWI_MASTER_H
#define TWI_MASTER_H
#include <TwoWireBase.h>

// I2C clock in Hz
#define F_TWI 400000L

//------------------------------------------------------------------------------
// Status codes in TWSR - names are from Atmel TWSR.h with TWSR_ added

// start condition transmitted
#define TWSR_START  0x08

// repeated start condition transmitted
#define TWSR_REP_START  0x10

// slave address plus write bit transmitted, ACK received
#define TWSR_MTX_ADR_ACK  0x18

// data transmitted, ACK received
#define TWSR_MTX_DATA_ACK  0x28

// slave address plus read bit transmitted, ACK received
#define TWSR_MRX_ADR_ACK  0x40

//------------------------------------------------------------------------------
class TwiMaster : public TwoWireBase {
  uint8_t status_;
  void execCmd(uint8_t cmdReg);
public:
  /** init hardware TWI */
  void init(uint8_t enablePullup);
  
  /** read byte with Ack */
  uint8_t readAck(void);
  
  /** read byte with Nak */
  uint8_t readNak(void);
  
  /** read a byte and send Ack if last is false else Nak to terminate read */
  uint8_t read(uint8_t last) {return last ? readNak() : readAck();}
  
  /** send new address and read/write bit without stop */
  uint8_t restart(uint8_t addressRW) {return start(addressRW);}
  uint8_t restart(uint8_t address, uint8_t rw) {return restart((address << 1) | rw); }
  
  /** issue a start condition for i2c address with read/write bit */
  uint8_t start(uint8_t addressRW);
  uint8_t start(uint8_t address, uint8_t rw) {return start((address << 1) | rw); }
  
  /** return status */
  uint8_t status(void) {return status_;}
  
  /** issue a stop condition */
  void stop(void);
  
  /** write a byte and return true for Ack or false for Nak */
  uint8_t write(uint8_t data);
};

#endif //TWI_MASTER_H
