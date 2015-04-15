/**************************************************************************/
/*! 
    @file     Adafruit_PN532.cpp
    @author   Adafruit Industries
    @license  BSD (see license.txt)
	
	  Driver for NXP's PN532 NFC/13.56MHz RFID Transceiver

	  This is a library for the Adafruit PN532 NFC/RFID breakout boards
	  This library works with the Adafruit NFC breakout 
	  ----> https://www.adafruit.com/products/364
	
	  Check out the links above for our tutorials and wiring diagrams 
	  These chips use SPI or I2C to communicate.
	
	  Adafruit invests time and resources providing this open source code, 
	  please support Adafruit and open-source hardware by purchasing 
	  products from Adafruit!

    @section  HISTORY

    v2.1 - Added NTAG2xx helper functions
    
    v2.0 - Refactored to add I2C support from Adafruit_NFCShield_I2C library.

    v1.4 - Added setPassiveActivationRetries()
	
    v1.2 - Added writeGPIO()
         - Added readGPIO()

    v1.1 - Changed readPassiveTargetID() to handle multiple UID sizes
         - Added the following helper functions for text display
             static void PrintHex(const byte * data, const uint32_t numBytes)
             static void PrintHexChar(const byte * pbtData, const uint32_t numBytes)
         - Added the following Mifare Classic functions:
             bool mifareclassic_IsFirstBlock (uint32_t uiBlock)
             bool mifareclassic_IsTrailerBlock (uint32_t uiBlock)
             uint8_t mifareclassic_AuthenticateBlock (uint8_t * uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t * keyData)
             uint8_t mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t * data)
             uint8_t mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t * data)
         - Added the following Mifare Ultalight functions:
             uint8_t mifareultralight_ReadPage (uint8_t page, uint8_t * buffer)	
*/
/**************************************************************************/
#include "nfcPN532.h"

byte pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
byte pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};

// Uncomment these lines to enable debug output for PN532(SPI) and/or MIFARE related code
// #define PN532DEBUG
// #define MIFAREDEBUG

// Hardware SPI-specific configuration:
#define PN532_SPI_SETTING SPISettings(1000000, LSBFIRST, SPI_MODE0)
#define PN532_SPI_CLOCKDIV SPI_CLOCK_DIV16

#define PN532_PACKBUFFSIZ 64
byte pn532_packetbuffer[PN532_PACKBUFFSIZ];

#ifndef _BV
    #define _BV(bit) (1<<(bit))
#endif


/**************************************************************************/
/*! 
    @brief  Instantiates a new PN532 class using hardware SPI.

    @param  ss        SPI chip select pin (CS/SSEL)
*/
/**************************************************************************/
uint8_t _ss;
#define LOW 0
#define HIGH 1
#define true 1
#define false 0
uint8_t _inListedTag;

void Adafruit_PN532_Adafruit_PN532(uint8_t ss){
 /* _clk(0),
  _miso(0),
  _mosi(0),
  _ss(ss),
  _irq(0),
  _reset(0),
  _usingSPI(true),
  _hardwareSPI(true)
{
  pinMode(_ss, OUTPUT);
}*/
}
/**************************************************************************/
/*! 
    @brief  Setups the HW
*/
/**************************************************************************/
void Adafruit_PN532_begin(void) {
	SPI_begin();
	//SPI_setDataMode(SPI_MODE0);
	//SPI_setBitOrder(LSBFIRST);
	//SPI_setClockDivider(PN532_SPI_CLOCKDIV);

	#ifdef SPI_HAS_TRANSACTION
		//SPI_beginTransaction(PN532_SPI_SETTING);
	#endif

	digitalWrite(_ss, LOW);
	
	delay(1000);

	// not exactly sure why but we have to send a dummy command to get synced up
	pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
	Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer, 1, 1000);

	// ignore response!

	digitalWrite(_ss, HIGH);
	#ifdef SPI_HAS_TRANSACTION
		SPI_endTransaction();
	#endif
}

/**************************************************************************/
/*! 
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
*/
/**************************************************************************/
uint32_t Adafruit_PN532_getFirmwareVersion(void) {
  uint32_t response;

  pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
  
  if (! Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer, 1, 1000))
    return 0;
  
  // read data packet
  Adafruit_PN532_readdata(pn532_packetbuffer, 12);
  
  // check some basic stuff
  if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6)) {
    #ifdef PN532DEBUG
      Serial.println(F("Firmware doesn't match!"));
    #endif
    return 0;
  }
  
  int offset = 6;  // Skip a response byte when using I2C to ignore extra data.
  response = pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];

  return response;
}


/**************************************************************************/
/*! 
    @brief  Sends a command and waits a specified period for the ACK

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    The size of the command in bytes 
    @param  timeout   timeout before giving up
    
    @returns  1 if everything is OK, 0 if timeout occured before an
              ACK was recieved
*/
/**************************************************************************/
// default timeout of one second
bool Adafruit_PN532_sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
  uint16_t timer = 0;
  
  // write the command
  Adafruit_PN532_writecommand(cmd, cmdlen);
  
  // Wait for chip to say its ready!
  if (!Adafruit_PN532_waitready(timeout)) {
    return false;
  }

  #ifdef PN532DEBUG
    if (!_usingSPI) {
      Serial.println(F("IRQ received"));
    }
  #endif
  
  // read acknowledgement
  if (!Adafruit_PN532_readack()) {
    #ifdef PN532DEBUG
      Serial.println(F("No ACK frame received!"));
    #endif
    return false;
  }

  // For SPI only wait for the chip to be ready again.
  // This is unnecessary with I2C.
  if (!Adafruit_PN532_waitready(timeout)) {
    return false;
  }
  
  return true; // ack'd command
}


/**************************************************************************/
/*! 
    @brief  Configures the SAM (Secure Access Module)
*/
/**************************************************************************/
bool Adafruit_PN532_SAMConfig(void) {
  pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  pn532_packetbuffer[1] = 0x01; // normal mode;
  pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
  pn532_packetbuffer[3] = 0x01; // use IRQ pin!
  
  if (! Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer, 4, 1000))
    return false;

  // read data packet
  Adafruit_PN532_readdata(pn532_packetbuffer, 8);
  
  int offset = 5;
  return  (pn532_packetbuffer[offset] == 0x15);
}

/**************************************************************************/
/*! 
    Sets the MxRtyPassiveActivation byte of the RFConfiguration register
    
    @param  maxRetries    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool Adafruit_PN532_setPassiveActivationRetries(uint8_t maxRetries) {
  pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
  pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
  pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
  pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
  pn532_packetbuffer[4] = maxRetries;

  #ifdef MIFAREDEBUG
    Serial.print(F("Setting MxRtyPassiveActivation to ")); Serial.print(maxRetries, DEC); Serial.println(F(" "));
  #endif
  
  if (! Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer, 5, 1000))
    return 0x0;  // no ACK
  
  return 1;
}

/***** ISO14443A Commands ******/

/**************************************************************************/
/*! 
    Waits for an ISO14443A target to enter the field
    
    @param  cardBaudRate  Baud rate of the card
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool Adafruit_PN532_readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
  pn532_packetbuffer[2] = cardbaudrate;
  
  if (!Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer, 3, timeout))
  {
    #ifdef PN532DEBUG
      Serial.println(F("No card(s) read"));
    #endif
    return 0x0;  // no cards read
  }

  // wait for a card to enter the field (only possible with I2C)
    
  // read data packet
  Adafruit_PN532_readdata(pn532_packetbuffer, 20);
  // check some basic stuff

  /* ISO14443A card response should be in the following format:
  
    byte            Description
    -------------   ------------------------------------------
    b0..6           Frame header and preamble
    b7              Tags Found
    b8              Tag Number (only one used in this example)
    b9..10          SENS_RES
    b11             SEL_RES
    b12             NFCID Length
    b13..NFCIDLen   NFCID                                      */
  
  #ifdef MIFAREDEBUG
    Serial.print(F("Found ")); Serial.print(pn532_packetbuffer[7], DEC); Serial.println(F(" tags"));
  #endif
  if (pn532_packetbuffer[7] != 1) 
    return 0;
    
  uint16_t sens_res = pn532_packetbuffer[9];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[10];
  #ifdef MIFAREDEBUG
    Serial.print(F("ATQA: 0x"));  Serial.println(sens_res, HEX); 
    Serial.print(F("SAK: 0x"));  Serial.println(pn532_packetbuffer[11], HEX); 
  #endif
  
  /* Card appears to be Mifare Classic */
  *uidLength = pn532_packetbuffer[12];
  #ifdef MIFAREDEBUG
    Serial.print(F("UID:")); 
  #endif
  for (uint8_t i=0; i < pn532_packetbuffer[12]; i++) 
  {
    uid[i] = pn532_packetbuffer[13+i];
    #ifdef MIFAREDEBUG
      Serial.print(F(" 0x"));Serial.print(uid[i], HEX); 
    #endif
  }
  #ifdef MIFAREDEBUG
    Serial.println();
  #endif

  return 1;
}

/**************************************************************************/
/*! 
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
/**************************************************************************/
bool Adafruit_PN532_inDataExchange(uint8_t * send, uint8_t sendLength, uint8_t * response, uint8_t * responseLength) {
  if (sendLength > PN532_PACKBUFFSIZ-2) {
    #ifdef PN532DEBUG
      Serial.println(F("APDU length too long for packet buffer"));
    #endif
    return false;
  }
  uint8_t i;
  
  pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = _inListedTag;
  for (i=0; i<sendLength; ++i) {
    pn532_packetbuffer[i+2] = send[i];
  }
  
  if (!Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer,sendLength+2,1000)) {
    #ifdef PN532DEBUG
      Serial.println(F("Could not send ADPU"));
    #endif
    return false;
  }

  if (!Adafruit_PN532_waitready(1000)) {
    #ifdef PN532DEBUG
      Serial.println(F("Response never received for ADPU..."));
    #endif
    return false;
  }

  Adafruit_PN532_readdata(pn532_packetbuffer,sizeof(pn532_packetbuffer));
  
  if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0 && pn532_packetbuffer[2] == 0xff) {
    uint8_t length = pn532_packetbuffer[3];
    if (pn532_packetbuffer[4]!=(uint8_t)(~length+1)) {
      #ifdef PN532DEBUG
        Serial.println(F("Length check invalid"));
        Serial.println(length,HEX);
        Serial.println((~length)+1,HEX);
      #endif
      return false;
    }
    if (pn532_packetbuffer[5]==PN532_PN532TOHOST && pn532_packetbuffer[6]==PN532_RESPONSE_INDATAEXCHANGE) {
      if ((pn532_packetbuffer[7] & 0x3f)!=0) {
        #ifdef PN532DEBUG
          Serial.println(F("Status code indicates an error"));
        #endif
        return false;
      }
      
      length -= 3;
      
      if (length > *responseLength) {
        length = *responseLength; // silent truncation...
      }
      
      for (i=0; i<length; ++i) {
        response[i] = pn532_packetbuffer[8+i];
      }
      *responseLength = length;
      
      return true;
    } 
    else {
      return false;
    } 
  } 
  else {
    return false;
  }
}

/**************************************************************************/
/*! 
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
*/
/**************************************************************************/
bool Adafruit_PN532_inListPassiveTarget(void) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;
  pn532_packetbuffer[2] = 0;
  
  #ifdef PN532DEBUG 
    Serial.print(F("About to inList passive target"));
  #endif

  if (!Adafruit_PN532_sendCommandCheckAck(pn532_packetbuffer,3,1000)) {
    #ifdef PN532DEBUG
      Serial.println(F("Could not send inlist message"));
    #endif
    return false;
  }

  if (!Adafruit_PN532_waitready(30000)) {
    return false;
  }

  Adafruit_PN532_readdata(pn532_packetbuffer,sizeof(pn532_packetbuffer));
  
  if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0 && pn532_packetbuffer[2] == 0xff) {
    uint8_t length = pn532_packetbuffer[3];
    if (pn532_packetbuffer[4]!=(uint8_t)(~length+1)) {
      #ifdef PN532DEBUG
        Serial.println(F("Length check invalid"));
        Serial.println(length,HEX);
        Serial.println((~length)+1,HEX);
      #endif
      return false;
    }
    if (pn532_packetbuffer[5]==PN532_PN532TOHOST && pn532_packetbuffer[6]==PN532_RESPONSE_INLISTPASSIVETARGET) {
      if (pn532_packetbuffer[7] != 1) {
        #ifdef PN532DEBUG
        Serial.println(F("Unhandled number of targets inlisted"));
        #endif
        return false;
      }
      
      _inListedTag = pn532_packetbuffer[8];
      
      return true;
    } else {
      #ifdef PN532DEBUG
        Serial.print(F("Unexpected response to inlist passive host"));
      #endif
      return false;
    } 
  } 
  else {
    #ifdef PN532DEBUG
      Serial.println(F("Preamble missing"));
    #endif
    return false;
  }

  return true;
}


/************** high level communication functions (handles both I2C and SPI) */


/**************************************************************************/
/*! 
    @brief  Tries to read the SPI or I2C ACK signal
*/
/**************************************************************************/
bool Adafruit_PN532_readack(void) {
  uint8_t ackbuff[6];
  
  Adafruit_PN532_readdata(ackbuff, 6);
  
  return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 6));
}


/**************************************************************************/
/*! 
    @brief  Return true if the PN532 is ready with a response.
*/
/**************************************************************************/
bool Adafruit_PN532_isready(void) {
	// SPI read status and check if ready.
	#ifdef SPI_HAS_TRANSACTION
		if (_hardwareSPI) SPI.beginTransaction(PN532_SPI_SETTING);
	#endif
	digitalWrite(_ss, LOW);
	delay(2); 
	Adafruit_PN532_spi_write(PN532_SPI_STATREAD);
	// read byte
	uint8_t x = Adafruit_PN532_spi_read();
	
	digitalWrite(_ss, HIGH);
	#ifdef SPI_HAS_TRANSACTION
		if (_hardwareSPI) SPI.endTransaction();
	#endif

	// Check if status is ready.
	return x == PN532_SPI_READY;
}

/**************************************************************************/
/*! 
    @brief  Waits until the PN532 is ready.

    @param  timeout   Timeout before giving up
*/
/**************************************************************************/
bool Adafruit_PN532_waitready(uint16_t timeout) {
  uint16_t timer = 0;
  while(!Adafruit_PN532_isready()) {
    if (timeout != 0) {
      timer += 10;
      if (timer > timeout) {
        return false;
      }
    }
    delay(10);
  }
  return true;
}

/**************************************************************************/
/*! 
    @brief  Reads n bytes of data from the PN532 via SPI or I2C.

    @param  buff      Pointer to the buffer where data will be written
    @param  n         Number of bytes to be read
*/
/**************************************************************************/
void Adafruit_PN532_readdata(uint8_t* buff, uint8_t n) {
	// SPI write.
	#ifdef SPI_HAS_TRANSACTION
		if (_hardwareSPI) SPI.beginTransaction(PN532_SPI_SETTING);
	#endif
	digitalWrite(_ss, LOW);
	delay(2); 
	Adafruit_PN532_spi_write(PN532_SPI_DATAREAD);

	#ifdef PN532DEBUG
		Serial.print(F("Reading: "));
	#endif
	for (uint8_t i=0; i<n; i++) {
		delay(1);
		buff[i] = Adafruit_PN532_spi_read();
		#ifdef PN532DEBUG
			Serial.print(F(" 0x"));
			Serial.print(buff[i], HEX);
		#endif
	}

	#ifdef PN532DEBUG
		Serial.println();
	#endif

	digitalWrite(_ss, HIGH);
	#ifdef SPI_HAS_TRANSACTION
		if (_hardwareSPI) SPI.endTransaction();
	#endif
}

/**************************************************************************/
/*! 
    @brief  Writes a command to the PN532, automatically inserting the
            preamble and required frame details (checksum, len, etc.)

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    Command length in bytes 
*/
/**************************************************************************/
void Adafruit_PN532_writecommand(uint8_t* cmd, uint8_t cmdlen) {
	// SPI command write.
	uint8_t checksum;

	cmdlen++;
	
	#ifdef PN532DEBUG
		Serial.print(F("\nSending: "));
	#endif

	#ifdef SPI_HAS_TRANSACTION
		if (_hardwareSPI) SPI.beginTransaction(PN532_SPI_SETTING);
	#endif
	digitalWrite(_ss, LOW);
	delay(2);     // or whatever the delay is for waking up the board
	Adafruit_PN532_spi_write(PN532_SPI_DATAWRITE);

	checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
	Adafruit_PN532_spi_write(PN532_PREAMBLE);
	Adafruit_PN532_spi_write(PN532_PREAMBLE);
	Adafruit_PN532_spi_write(PN532_STARTCODE2);

	Adafruit_PN532_spi_write(cmdlen);
	Adafruit_PN532_spi_write(~cmdlen + 1);
 
	Adafruit_PN532_spi_write(PN532_HOSTTOPN532);
	checksum += PN532_HOSTTOPN532;

	#ifdef PN532DEBUG
		Serial.print(F(" 0x")); Serial.print(PN532_PREAMBLE, HEX);
		Serial.print(F(" 0x")); Serial.print(PN532_PREAMBLE, HEX);
		Serial.print(F(" 0x")); Serial.print(PN532_STARTCODE2, HEX);
		Serial.print(F(" 0x")); Serial.print(cmdlen, HEX);
		Serial.print(F(" 0x")); Serial.print(~cmdlen + 1, HEX);
		Serial.print(F(" 0x")); Serial.print(PN532_HOSTTOPN532, HEX);
	#endif

	for (uint8_t i=0; i<cmdlen-1; i++) {
		Adafruit_PN532_spi_write(cmd[i]);
		checksum += cmd[i];
		#ifdef PN532DEBUG
			Serial.print(F(" 0x")); Serial.print(cmd[i], HEX);
		#endif
	}
	
	Adafruit_PN532_spi_write(~checksum);
	Adafruit_PN532_spi_write(PN532_POSTAMBLE);
	digitalWrite(_ss, HIGH);
	#ifdef SPI_HAS_TRANSACTION
		if (_hardwareSPI) SPI.endTransaction();
	#endif

	#ifdef PN532DEBUG
		Serial.print(F(" 0x")); Serial.print(~checksum, HEX);
		Serial.print(F(" 0x")); Serial.print(PN532_POSTAMBLE, HEX);
		Serial.println();
	#endif
} 
/************** low level SPI */

/**************************************************************************/
/*! 
    @brief  Low-level SPI write wrapper

    @param  c       8-bit command to write to the SPI bus
*/
/**************************************************************************/
void Adafruit_PN532_spi_write(uint8_t c) {
	// Hardware SPI write.
	SPI_transfer(c);
}

/**************************************************************************/
/*! 
    @brief  Low-level SPI read wrapper

    @returns The 8-bit value that was read from the SPI bus
*/
/**************************************************************************/
uint8_t Adafruit_PN532_spi_read(void) {
  int8_t i, x;
  x = 0;

	// Hardware SPI read.
	x = SPI_transfer(0x00);

  return x;
}
