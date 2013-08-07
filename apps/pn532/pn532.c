#include "pn532.h"
#include "i2c_sync.h"

// Private methods

PN532_Error PN532_SendCommandFrame(uint8_t* data, uint8_t dataLen);
PN532_Error PN532_ReceiveAckFrame();
PN532_Error PN532_ReceiveDataFrame(uint8_t* data, uint8_t* dataLen);
PN532_Error PN532_SendAckFrame();
I2C_STATUS PN532_I2C_WriteReadSync(uint8_t sendLen, uint8_t* sendData, uint8_t receiveLen, uint8_t* receiveData);


I2C_State pn532I2CState;

#define PN532_ADDRESSING_RETRIES 100
#define PN532_READY_RETRIES 1000
#define PN532_MAX_BUF_LEN 200
#define PN532_ADDRESSING_RETRY_DELAY 10000
#define PN532_READY_RETRY_DELAY 100000
#define PN532_WAKEUP_DELAY 10000
#define PN532_CMD_OUT_DELAY 100000
#define PN532_ACK_IN_DELAY 100000
#define PN532_DATA_IN_DELAY 1000000
#define PN532_ACK_OUT_DELAY 100000


extern volatile uint8_t latestI2CState;

void PN532_Sleep(uint32_t len) {
	volatile uint32_t i = 0;
	while (i < len) { i++; }
}

PN532_Error PN532_Init() {
	I2C_Init(I2C_MODE_STANDARD, &pn532I2CState); 

/*
	uint8_t cmd = PN532_Cmd_GetFirmwareVersion;	//Wake up by sending address
	I2C_STATUS i2cErr = PN532_I2C_WriteReadSync(1, &cmd, 0, NULL);
	if (i2cErr) return 0x10 + i2cErr;
	PN532_Sleep(PN532_WAKEUP_DELAY);
*/
	return PN532_Err_OK;
}

PN532_Error PN532_SAMConfiguration() {
	uint8_t cmd[] = {PN532_Cmd_SAMConfiguration, 1, 1, 0};
	PN532_Error err = PN532_SendCommand(cmd, 4, NULL, NULL);
	if (err != PN532_Err_OK) return err;
	return PN532_Err_OK;
}

PN532_Error PN532_GetFirmwareVersion(uint32_t* outFirmwareVersion) {
	uint8_t cmd = PN532_Cmd_GetFirmwareVersion;
	uint8_t responseLen = 4;
	PN532_Error err = PN532_SendCommand(&cmd, 1, (uint8_t*)outFirmwareVersion, &responseLen);
	if (err != PN532_Err_OK) return err;
	if (responseLen != 4) return PN532_Err_InvalidResponse;
	return PN532_Err_OK;
}

PN532_Error PN532_ListCards(int* count, PN532_Modulation modulation) {
	uint8_t cmd[] = {PN532_Cmd_InListPassiveTarget, 1, modulation};
	uint8_t responseLen = 50;
	uint8_t response[50];
	PN532_Error err = PN532_SendCommand(cmd, 3, response, &responseLen);
	*count = 0;
	if (err != PN532_Err_OK) return err;
	if (responseLen < 3) return PN532_Err_InvalidResponse;
	*count = 0x80 + responseLen; //response[2];
	return PN532_Err_OK;
}

PN532_Error PN532_SendCommand(uint8_t* data, uint16_t dataLen, uint8_t* response, uint8_t* responseLen) {

	PN532_Sleep(PN532_CMD_OUT_DELAY);
	PN532_Error err = PN532_SendCommandFrame(data, dataLen);
	if (err) return 0x10 + err;
	PN532_Sleep(PN532_ACK_IN_DELAY);
	err = PN532_ReceiveAckFrame();
	if (err) return 0x20 + err;

	if (response && responseLen) {
		PN532_Sleep(PN532_DATA_IN_DELAY);
		err = PN532_ReceiveDataFrame(response, responseLen);
		if (err) return 0x30 + err;
		PN532_Sleep(PN532_ACK_OUT_DELAY);
		err = PN532_SendAckFrame();
		if (err) return 0x40 + err;
	}

	return PN532_Err_OK;
}


PN532_Error PN532_SendCommandFrame(uint8_t* data, uint8_t dataLen) {
	uint8_t cmdFrameLen = dataLen+8;
	if (cmdFrameLen > PN532_MAX_BUF_LEN) return PN532_Err_PayloadTooLong;
	uint8_t buf[cmdFrameLen];
	uint8_t dir = PN532_Dir_HostToChip;

	//Write command frame
	buf[0] = 0x00;		//Preamble
	buf[1] = 0x00;		//start code
	buf[2] = 0xff;		//start code
	buf[3] = dataLen+1;		//len
	buf[4] = 1 + ~buf[3];	//lcs
	buf[5] = dir;			//tfi
	uint8_t dcs = dir;		//data checksum calculation
	int i;
	for (i=0;i<dataLen;i++) {
		buf[6+i] = data[i];
		dcs += data[i];		//pd[n]
	}
	buf[dataLen+6] = 1 + ~dcs;	//dcs
	buf[dataLen+7] = 0x00;	//postamble

	I2C_STATUS i2cErr = PN532_I2C_WriteReadSync(cmdFrameLen, buf, 0, NULL);
	if (i2cErr) return 0x08 + i2cErr;//PN532_Err_Transport_Failure;
	return PN532_Err_OK;
}

PN532_Error PN532_ReceiveAckFrame() {
	uint8_t buf[7];
	uint32_t retry;
	for (retry = 0; retry < PN532_READY_RETRIES; retry++) {
		I2C_STATUS i2cErr = PN532_I2C_WriteReadSync(0, NULL, 7, buf);
		if (i2cErr) return 0x08 + i2cErr;//PN532_Err_Transport_Failure;

		//I2C level seems to be ok. Check ready (PN532 protocol extension for I2C)
		if (buf[0] & 1) {
			//Valid frame. Check preamble, frame start, postamble
			if ((buf[1] != 0x00) || (buf[2] != 0x00) || (buf[3] != 0xff) || (buf[6] != 0x00)) return PN532_Err_Framing;

			//Check ACK
			if ((buf[4] != 0x00) || (buf[5] != 0xff)) return PN532_Err_Nacked;
			return PN532_Err_OK;	//All ok, got valid ACK	
		}
		PN532_Sleep(PN532_READY_RETRY_DELAY);
	} 
	return PN532_Err_Timeout;
}

PN532_Error PN532_ReceiveDataFrame(uint8_t* data, uint8_t* dataLen) {
	uint32_t retry;
	uint8_t buf[PN532_MAX_BUF_LEN];
	for (retry = 0; retry < PN532_READY_RETRIES; retry++) {
		I2C_STATUS i2cErr = PN532_I2C_WriteReadSync(0, NULL, PN532_MAX_BUF_LEN, buf);
		if (i2cErr) return 0x08 + i2cErr;//PN532_Err_Transport_Failure;
		if (buf[0] & 1) {
			//Check preamble, frame start
			if ((buf[1] != 0x00) || (buf[2] != 0x00) || (buf[3] != 0xff) || (buf[6] != PN532_Dir_ChipToHost)) {
				return PN532_Err_Framing;
			}

			uint8_t len = buf[4]-2;	//We don't need TFI and the response's command code
			//TODO: Check LCS, extended frame, DCS ***********
			if (len > *dataLen) return PN532_Err_PayloadTooLong;
			uint8_t i;
			for (i=0; i<len; i++) {
				data[i] = buf[i+8];
			}
			*dataLen = len;
			return PN532_Err_OK;
		}
		PN532_Sleep(PN532_READY_RETRY_DELAY);
	}
	return PN532_Err_Timeout;
}

PN532_Error PN532_SendAckFrame() {
	uint8_t buf[6] = { 0x00, 0x00, 0xff, 0x00, 0xff, 0x00};
	I2C_STATUS i2cErr = PN532_I2C_WriteReadSync(6, buf, 0, NULL);
	if (i2cErr) return 0x08 + i2cErr;//PN532_Err_Transport_Failure;
	return PN532_Err_OK;
}

I2C_STATUS PN532_I2C_WriteReadSync(uint8_t sendLen, uint8_t* sendData, uint8_t receiveLen, uint8_t* receiveData) {
	uint32_t retry;
	for (retry = 0; retry < PN532_ADDRESSING_RETRIES; retry++) {
		I2C_STATUS i2cErr = I2C_WriteReadSync(PN532_I2C_ADDRESS, sendLen, sendData, receiveLen, receiveData);
		if (i2cErr == I2C_STATUS_ADDRESSING_FAILED) {
			PN532_Sleep(PN532_ADDRESSING_RETRY_DELAY);
			continue;
		}
		if (i2cErr) return PN532_Err_Transport_Failure;
		return I2C_STATUS_OK;
	}
	return I2C_STATUS_TIMEOUT;
}

/*

normal information frame (command, response)

0x00 0x00 0xFF LEN LCS TFI PD[n] DCS 0x00
0x00: Preamble
0x00 0xFF: Start code
LEN: Length of TFI and PD[n] in bytes
LCS: Length checksum (LEN+LCS = 0x00)
TFI: 0xD4: Host controller to PN532, 0xD5: PN532 to host controller
PD[n]: LEN-1 bytes of data
DCS: Data checksum: (TFI+PD[0..n]+DCS = 0x00)
0x00: Postamble

ACK frame

0x00 0x00 0xff 0x00 0xff 0x00
0x00: Preamble
0x00 0xff: Start code
0x00 0xff: ACK
0x00: Postamble

NACK frame

0x00 0x00 0xff 0xff 0x00 0x00
0x00: Preamble
0x00 0xff: Start code
0xff 0x00: NACK
0x00: Postamble

In I2C mode, all frames from chip to host are prefixed with a status byte.
Bit 0: Ready. If 1, remaining contents are ok. If 0, other data is invalid. Host should poll again.


*/

