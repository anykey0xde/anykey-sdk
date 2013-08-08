/** Library for the NXP PN532 RFID IC. The PN532 should be connected via I2C. Handshake via interrupt is not used. */

#ifndef _PN532_
#define _PN532_

#include "anykey/anykey.h"

#define PN532_I2C_ADDRESS 0x24
/* spec says 0x48/0x49 - seems to be shifted and include read/write bit  */

typedef enum {
	PN532_Cmd_Diagnose = 0x00,
	PN532_Cmd_GetFirmwareVersion = 0x02,
	PN532_Cmd_GetGeneralStatus = 0x04,
	PN532_Cmd_ReadRegister = 0x06,
	PN532_Cmd_WriteRegister = 0x08,
	PN532_Cmd_ReadGPIO = 0x0c,
	PN532_Cmd_WriteGPIO = 0x0e,
	PN532_Cmd_SetSerialBaudRate = 0x10,
	PN532_Cmd_SetParameters = 0x12,
	PN532_Cmd_SAMConfiguration = 0x14,
	PN532_Cmd_PowerDown = 0x16,
	PN532_Cmd_RFConfiguration = 0x32,
	PN532_Cmd_RFRegulationTest = 0x58,	
	PN532_Cmd_InJumpForDEP = 0x56,
	PN532_Cmd_InJumpForPSL = 0x46,
	PN532_Cmd_InListPassiveTarget = 0x4a,
	PN532_Cmd_InATR = 0x50,
	PN532_Cmd_InPSL = 0x4e,
	PN532_Cmd_InDataExchange = 0x40,
	PN532_Cmd_InCommunicateThru = 0x42,
	PN532_Cmd_InDeselect = 0x44,
	PN532_Cmd_InRelease = 0x52,
	PN532_Cmd_InSelect = 0x54,
	PN532_Cmd_InAutoPoll = 0x60,
	PN532_Cmd_TgInitAsTarget = 0x8c,
	PN532_Cmd_TgSetGeneralBytes = 0x92,
	PN532_Cmd_TgGetData = 0x86,
	PN532_Cmd_TgSetData = 0x8e,
	PN532_Cmd_TgSetMetaData = 0x94,
	PN532_Cmd_TgGetInitiatorCommand = 0x88,
	PN532_Cmd_TgResponseToInitiator = 0x90,
	PN532_Cmd_TgGetTargetStatus = 0x8a
} PN532_Command;

typedef enum {
	PN532_Dir_HostToChip = 0xd4,
	PN532_Dir_ChipToHost = 0xd5
} PN532_Direction;

typedef enum {
	PN532_Mod_TypeA106kbps = 0x00,
	PN532_Mod_FeliCa206kbps = 0x01,
	PN532_Mod_FeliCa424kbps = 0x02,
	PN532_Mod_TypeB106kbps = 0x03,
	PN532_Mod_InnovisionJewel106kbps = 0x04
} PN532_Modulation;


typedef enum {
	PN532_Err_OK = 0,
	PN532_Err_Transport_Failure = 1,	//Error on the underlying transport layer
	PN532_Err_Framing = 2,				//preamble / postamble / start code not ok
	PN532_Err_Timeout = 3,				//An operation timed out
	PN532_Err_Nacked = 4,				//A sent frame was not acknowledged
	PN532_Err_PayloadTooLong = 5,		//The driver could not handle the frame length
	PN532_Err_InvalidResponse = 6		//Unexpected payload data was returned
} PN532_Error;

/** initializes the PN532
@return OK on success or an error code on failure. */
PN532_Error PN532_Init();

/** sets up the SAM configuration to a usable state. 
@return OK on success or an error code on failure. */
PN532_Error PN532_SAMConfiguration();

/** queries the chip's general status
@param a pointer to return an error code. Pass NULL if not interested.
@param a pointer to return if a field is presenet. Pass NULL if not interested.
@param a pointer to return the number of current targets. Pass NULL if not interested.
@return an error code */
PN532_Error PN532_GetGeneralStatus(uint8_t* errCode, bool* field, uint8_t* numTargets);

/** returns the firmware version
@param pointer to return the version information. Bytes:
0: IC version
1: firmware version
2: firmware revision
3: supported options */
PN532_Error PN532_GetFirmwareVersion(uint32_t* outFirmwareVersion);

/** enumerates currently available cards 
@param max pointer to number of cards. In: Maximum number to detect, out: number of found cards or 0 on err
@param modulation modulation to use 
@return error code */
PN532_Error PN532_ListCards(int* max, PN532_Modulation modulation);

/** sends a frame to the card
@param data payload data
@param dataLen length of data payload
@param response response buffer (to be filled with data payload, not the full response frame). Pass NULL to skip response phase.
@param pointer to response buffer length. Before: Max size, after: Actual size. Pass NULL to skip response phase.
œ
@return error code */
PN532_Error PN532_SendCommand(uint8_t* data, uint16_t dataLen, uint8_t* response, uint8_t* responseLen);

#endif