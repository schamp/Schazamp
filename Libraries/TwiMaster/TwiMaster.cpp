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
#include <avr/io.h>
#include <TwiMaster.h>
//------------------------------------------------------------------------------
void TwiMaster::execCmd(uint8_t cmdReg)
{
  TWCR = cmdReg;
  // wait for command to complete
  ///////////////put timeout here?
  while (!(TWCR & (1 << TWINT)));
	// status bits.
	status_ = TWSR & 0xF8; 
}
//------------------------------------------------------------------------------
// init hardware TWI
void TwiMaster::init(uint8_t enablePullup)
{
  // no prescaler
  TWSR = 0;
  // set bit rate factor
  TWBR = (F_CPU/F_TWI - 16)/2;
  
  if (!enablePullup) return; 
  
#if defined(__AVR_ATmega1280__)
  //Mega Arduino
  PORTD |= (1 << 0);
  PORTD |= (1 << 1);
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
  // Sanguino
  PORTC |= (1 << 0);
  PORTC |= (1 << 1);
#else // __AVR_ATmega1280__
  // all other Arduinos
  PORTC |= (1 << 4);
  PORTC |= (1 << 5);
#endif // __AVR_ATmega1280__
}
//------------------------------------------------------------------------------
// read byte with Ack
uint8_t TwiMaster::readAck(void)
{
	execCmd((1 << TWINT) | (1 << TWEN) | (1 << TWEA));   
  return TWDR;
}
//------------------------------------------------------------------------------
// read byte with Nak
uint8_t TwiMaster::readNak(void)
{
	execCmd((1 << TWINT) | (1 << TWEN));   
  return TWDR;
}
//------------------------------------------------------------------------------
// issue a start condition
uint8_t TwiMaster::start(uint8_t addressRW)
{
	// send START condition
	execCmd((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));
	if (status() != TWSR_START && status() != TWSR_REP_START) return 0;
	
	// send device address and direction
	TWDR = addressRW;
	execCmd((1 << TWINT) | (1 << TWEN));
	if (addressRW & I2C_READ) {
    return status() == TWSR_MRX_ADR_ACK;
  }
  else {
    return status() == TWSR_MTX_ADR_ACK;
  }
}
//------------------------------------------------------------------------------
// issue stop condition
void TwiMaster::stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	
	// wait until stop condition is executed and bus released
  ////////////////put timeout here?
	while(TWCR & (1 << TWSTO));
}
//------------------------------------------------------------------------------
// write a byte and return true for Ack or false for Nak
uint8_t TwiMaster::write(uint8_t data)
{
	TWDR = data;
	execCmd((1 << TWINT) | (1 << TWEN));
	return status() == TWSR_MTX_DATA_ACK;
}
