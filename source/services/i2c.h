#include <3ds.h>

Result i2cHidInit();
void i2cHidFinalize();
Result I2C_EnableRegisterBits8(uint8_t devid, uint8_t regid, uint8_t enablemask);
Result I2C_DisableRegisterBits8(uint8_t devid, uint8_t regid, uint8_t disablemask);
Result I2C_WriteRegister8(uint8_t devid, uint8_t regid, uint8_t regdata);
Result I2C_ReadRegisterBuffer8(uint8_t devid, uint8_t regid, uint8_t *buffer, size_t buffersize);
Result I2C_WriteRegisterBuffer(uint8_t devid, uint8_t regid, uint8_t *buffer, size_t buffersize);
Result I2C_ReadRegisterBuffer(uint8_t devid, uint8_t regid, uint8_t *buffer, size_t buffersize);