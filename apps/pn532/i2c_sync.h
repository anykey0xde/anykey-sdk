#ifndef _I2C_SYNC_
#define _I2C_SYNC_

#include "anykey/anykey.h"
#include "anykey/i2c.h"

//--------------------
//--- Blocking I2C ---
//--------------------

/** the following block implements blocking I2C transactions and retries. The API is asynchronous,
calls a callback when a transaction finished (on success and failure). The blocking approach is
not very elegant but simple to use. Note that we need to timeout to avoid blocking the processor -
Adjust the defined values depending on your peripheral's speed and transfer length. */

#define I2C_START_MAX_RETRIES 1000
#define I2C_FINISH_MAX_RETRIES 10000000

/** Blocking I2C transaction with retries */
I2C_STATUS I2C_WriteReadSync( uint8_t addr,
							uint16_t writeLen,
							uint8_t* writeBuf,
							uint16_t readLen,
							uint8_t* readBuf);

#endif