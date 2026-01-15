#include <3ds.h>
#include "mcuhid.hpp"
#include <cstring>
Handle mcuHidHandle;
Result mcuHidInit() {
    Result ret = 0;
    ret = srvGetServiceHandle(&mcuHidHandle, "mcu::HID");
    return ret;
}

void mcuHidExit() {
    svcCloseHandle(mcuHidHandle);
}

Result mcuHidEnableAccelerometer(uint8_t enable) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0x10040;
    cmdbuf[1] = enable;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    return cmdbuf[1];
}

Result mcuHidReadAccelerometerValues(AccelerometerEntry *entry) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0x60000;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    memcpy(entry, &cmdbuf[2], 6);
    return cmdbuf[1];
}

Result mcuHidGetSliderState(uint8_t *rawvalue) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0x70000;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    *rawvalue = cmdbuf[2] & 0xFF;
    return cmdbuf[1];
}

Result mcuHidGetAccelerometerEventHandle(Handle *handle) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0xC0000;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    *handle = cmdbuf[3];
    return cmdbuf[1];
}

Result mcuHidGetEventReason(uint32_t *reason) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0xD0000;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    *reason = cmdbuf[2];
    return cmdbuf[1];
}

Result mcuHidGetSoundVolume(uint8_t *volume) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0xE0000;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    *volume = cmdbuf[2] & 0xFF;
    return cmdbuf[1];
}

Result mcuHidEnableAccelerometerInterrupt(uint8_t enable) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0xF0040;
    cmdbuf[1] = enable;
    Result ret = svcSendSyncRequest(mcuHidHandle);

    if (R_FAILED(ret))
        return ret;

    return cmdbuf[1];
}