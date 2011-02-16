/****************************************************************************

Device      : ATmegaxxx

File name   : TWSR.h

Ver nr.     : 1.0

Description : Name definitions for TWSR values

Author      : Asmund Saetre

Change log  : Created 05.29.2000  AS
 
****************************************************************************/

#ifndef TWSR_h
#define TWSR_h
//General Master staus codes											
//***************************************************************************
#define START		              0x08		//START has been transmitted
#define	REP_START	            0x10		//Repeated START has been 
											//transmitted		
//Master Transmitter staus codes											
//***************************************************************************
#define	MTX_ADR_ACK		        0x18		//SLA+W has been tramsmitted
											//and ACK received	
#define	MTX_ADR_NACK	        0x20		//SLA+W has been tramsmitted
											//and NACK received		
#define	MTX_DATA_ACK	        0x28		//Data byte has been tramsmitted
											//and ACK received			
#define	MTX_DATA_NACK	        0x30		//Data byte has been tramsmitted
											//and NACK received			
#define	MTX_ARB_LOST	        0x38		//Arbitration lost in SLA+W or 
											//data bytes	
//Master Receiver staus codes	
//***************************************************************************
#define	MRX_ARB_LOST	        0x38		//Arbitration lost in SLA+R or 
											//NACK bit
#define	MRX_ADR_ACK		        0x40		//SLA+R has been tramsmitted
											//and ACK received	
#define	MRX_ADR_NACK	        0x48		//SLA+R has been tramsmitted
											//and NACK received		
#define	MRX_DATA_ACK	        0x50		//Data byte has been received
											//and ACK returned
#define	MRX_DATA_NACK	        0x58		//Data byte has been received
											//and NACK tramsmitted


//Slave Transmitter staus codes											
//***************************************************************************
#define	STX_ADR_ACK		        0xA8		//Own SLA+R has been received
											//and ACK returned
#define	ARB_LOST_STX_ADR_ACK    0xB0		//Arbitration lost in SLA+R/W as
                                            //a Master. Own SLA+W has been 
                                            //received and ACK returned
#define	STX_DATA_ACK	        0xB8		//Data byte has been tramsmitted
											//and ACK received			
#define	STX_DATA_NACK	        0xC0		//Data byte has been tramsmitted
											//and NACK received			
#define	STX_LAST_DATA 	        0xC8		//Last byte un I2DR has been 
                                            //transmitted(TWEA = '0') ACK has
                                            //been received											
//Slave Receiver staus codes	
//***************************************************************************
#define	SRX_ADR_ACK		        0x60		//SLA+R has been received
											//and ACK returned
#define	ARB_LOST_SRX_ADR_ACK	0x68		//Arbitration lost in SLA+R/W as
                                            //a Master. Own SLA+R has been 
                                            //received and ACK returned
#define	SRX_GCALL_ACK	        0x70		//Generall call has been received
											//and ACK returned
#define	ARB_LOST_SRX_GCALL_ACK	0x78		//Arbitration lost in SLA+R/W as
                                            //a Master. General Call has been 
                                            //received and ACK returned
#define	SRX_DATA_ACK	        0x80		//Previously addressed with own 
                                            //SLA+W.Data byte has been received
											//and ACK returned
#define	SRX_DATA_NACK	        0x88		//Previously addressed with own 
                                            //SLA+WData byte has been received
                                            //and NACK returned
#define	SRX_GCALL_ACK	        0x90		//Previously addressed with General 
                                            //Call.Data byte has been received
											//and ACK returned
#define	SRX_GCALL_NACK	        0x98		//Previously addressed with General 
                                            //Call. Data byte has been received
                                            //and NACK returned
#define	SRX_STOP	            0xA0		//A STOP condition or repeated START
                                            //condition has been received while 
                                            //still addressed as a slave
									        
//Miscellanous States
//***************************************************************************
#define	NO_INFO	                0xF8		//No relevant state information
                                            //TWINT = '0'
#define	BUS_ERROR	            0x00		//Bus error due to illegal START 
                                            //or STOP condition
#endif // TWRS_h
									        
									        						        