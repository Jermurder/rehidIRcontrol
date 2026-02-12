#pragma once
#include <3ds.h>
#include "../rings/AccelerometerRing.hpp" // For AccelerometerEntry

Result mcuHidInit();
void   mcuHidExit();
Result mcuHidGetSliderState(uint8_t *rawvalue);
Result mcuHidGetAccelerometerEventHandle(Handle *handle);
Result mcuHidEnableAccelerometerInterrupt(uint8_t enable);
Result mcuHidEnableAccelerometer(uint8_t enable);
Result mcuHidReadAccelerometerValues(AccelerometerEntry *entry);
Result mcuHidGetSoundVolume(uint8_t *volume);
Result mcuHidGetEventReason(uint32_t *reason);