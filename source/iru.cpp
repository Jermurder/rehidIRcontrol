#include <3ds.h>
#include "iru.hpp"

Handle iruhandle;
static int irurefcount;

Result iruInit_() {
    Result ret = 0;

    if (AtomicPostIncrement(&irurefcount))
        return -1;

    ret = srvGetServiceHandle(&iruhandle, "ir:u");

    if (R_FAILED(ret))
        goto cleanup0;

    ret = IRU_Initialize_();

    if (R_SUCCEEDED(ret))
        goto success;

    svcCloseHandle(iruhandle);
cleanup0:
    AtomicDecrement(&irurefcount);

success:
    return ret;
}

void iruExit_(void) {
    if (AtomicDecrement(&irurefcount))
        return;

    IRU_Shutdown_();
    svcCloseHandle(iruhandle);
    iruhandle = 0;
}

Result IRU_Initialize_(void) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x1, 0, 0); // 0x10000

    if (R_FAILED(ret = svcSendSyncRequest(iruhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}

uint32_t IRU_GetLatestKeysPA_() {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x7, 0, 0); // 0x70000

    if (R_FAILED(ret = svcSendSyncRequest(iruhandle)))
        return ret;

    return cmdbuf[1];
}

uint32_t IRU_GetStatePA_() {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x8, 0, 0); // 0x80000

    if (R_FAILED(ret = svcSendSyncRequest(iruhandle)))
        return ret;

    return cmdbuf[1];
}

Result IRU_Shutdown_(void) {
    Result ret = 0;
    uint32_t *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x2, 0, 0); // 0x20000

    if (R_FAILED(ret = svcSendSyncRequest(iruhandle)))
        return ret;

    ret = (Result)cmdbuf[1];

    return ret;
}
