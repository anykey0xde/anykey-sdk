#include "i2c_sync.h"


/** Note this is a signed type - we need to be able to set it to -1 (not set) */
volatile int i2cSyncStatus = I2C_STATUS_OK;


/** callback when a I2C transaction succeeded or failed */
void I2C_SyncReadCompletionHandler(uint32_t refcon, I2C_STATUS status) {
	i2cSyncStatus = status; //transaction completed - remember status for wait loop
}

I2C_STATUS I2C_WriteReadSync( uint8_t addr,
							uint16_t writeLen,
							uint8_t* writeBuf,
							uint16_t readLen,
							uint8_t* readBuf) {
	i2cSyncStatus = -1;
	int retries;
	I2C_STATUS status;

	//Try to start transaction
	for (retries = 0; retries < I2C_START_MAX_RETRIES; retries++) {
		status = I2C_WriteRead(addr, writeLen, writeBuf, readLen, readBuf, I2C_SyncReadCompletionHandler, 0);
		if (status == I2C_STATUS_OK) break;
	}		
	if (status != I2C_STATUS_OK) return status;

	for (retries = 0; retries < I2C_FINISH_MAX_RETRIES; retries++) {
		if (i2cSyncStatus >= 0) return (I2C_STATUS)i2cSyncStatus;
	}
	I2C_CancelTransaction();
	return I2C_STATUS_TIMEOUT;
}
