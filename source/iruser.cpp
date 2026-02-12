#include <3ds.h>
#include "csvc.h"
#include "iruser.hpp"

Handle iruserhandle;
static int iruserrefcount;

Result irUserInit() {
    if (AtomicPostIncrement(&iruserrefcount))
        return 0;

    Result ret = 0;

    if (R_FAILED(ret = srvGetServiceHandle(&iruserhandle, "ir:USE")))
        AtomicDecrement(&iruserrefcount);

    return ret;
}

void irUserFinalize() {
    AtomicDecrement(&iruserrefcount);
    svcCloseHandle(iruserhandle);
}

Result IRUSER_FinalizeIrnop(void) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x2, 0, 0); // 0x20000

    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_ClearReceiveBuffer(void) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x3, 0, 0); // 0x30000

    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_ClearSendBuffer(void) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x4, 0, 0); // 0x40000
    
    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_RequireConnection(uint8_t deviceId) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x6, 1, 0); // 0x60040
    cmdbuf[1] = deviceId;
    
    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_Disconnect(void) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x9, 0, 0); // 0x90000

    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_GetReceiveEvent(Handle *out) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0xA, 0, 0); // 0xA0000

    if(R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];
    *out = (Handle)cmdbuf[3];

    return ret;
}

Result IRUSER_GetConnectionStatusEvent(Handle *out) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0xC, 0, 0); // 0xC0000

    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];
    *out = (Handle)cmdbuf[3];

    return ret;
}

Result IRUSER_SendIrnop(const void *buffer, uint32_t size) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0xD, 1, 2); // 0xD0042
    cmdbuf[1] = size;
    cmdbuf[2] = (size << 14) | 2;
    cmdbuf[3] = (uint32_t)buffer;
    
    if(R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_InitializeIrnopShared(Handle shhandle, size_t sharedbufsize, size_t recvbufsize,
       size_t recvbufpacketcount, size_t sendbufsize, size_t sendbufpacketcount,
       uint8_t baudrate) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();
    
    cmdbuf[0] = IPC_MakeHeader(0x18, 6, 2); // 0x180182
    cmdbuf[1] = sharedbufsize;
    cmdbuf[2] = recvbufsize;
    cmdbuf[3] = recvbufpacketcount;
    cmdbuf[4] = sendbufsize;
    cmdbuf[5] = sendbufpacketcount;
    cmdbuf[6] = baudrate;
    cmdbuf[7] = 0;
    cmdbuf[8] = shhandle;

    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

Result IRUSER_ReleaseSharedData(uint32_t count) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x19, 1, 0); // 0x190040
    cmdbuf[1] = count;

    if (R_FAILED(ret = svcSendSyncRequest(iruserhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}