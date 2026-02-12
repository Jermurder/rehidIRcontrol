#include <new>
#include <cstdlib>
#include <3ds.h>
#include "irrst.hpp"

Handle irrstHandle_;
Handle irrstMemHandle_;
Handle irrstEvent_;
int irrstRefCount;

volatile uint32_t* irrstSharedMem_;

Result IRRST_GetHandles_(Handle* outMemHandle, Handle* outEventHandle) {
    uint32_t* cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x1, 0, 0); // 0x10000

    Result ret = 0;

    if (R_FAILED(ret = svcSendSyncRequest(irrstHandle_)))
        return ret;

    if (outMemHandle)
        * outMemHandle = cmdbuf[3];

    if (outEventHandle)
        * outEventHandle = cmdbuf[4];

    return cmdbuf[1];
}

Result IRRST_Initialize_(uint32_t unk1, uint8_t unk2) {
    uint32_t* cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x2, 2, 0); // 0x20080
    cmdbuf[1] = unk1;
    cmdbuf[2] = unk2;

    Result ret = 0;

    if (R_FAILED(ret = svcSendSyncRequest(irrstHandle_)))
        return ret;

    return cmdbuf[1];
}

Result IRRST_Shutdown_(void) {
    uint32_t* cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x3, 0, 0); // 0x30000

    Result ret = 0;

    if (R_FAILED(ret = svcSendSyncRequest(irrstHandle_)))
        return ret;

    return cmdbuf[1];
}

Result irrstInit_(uint8_t steal) {
    if (AtomicPostIncrement(&irrstRefCount))
        return 0;

    Result ret = 0;

    // Request service.
    if (R_FAILED(ret = srvGetServiceHandle(&irrstHandle_, "ir:rst")))
        goto cleanup0;

    // Get sharedmem handle.
    if (R_FAILED(ret = IRRST_GetHandles_(&irrstMemHandle_, &irrstEvent_)))
        goto cleanup1;

    // Initialize ir:rst
    if (envGetHandle("ir:rst") == 0)
        ret = IRRST_Initialize_(10, 0);

    // Map ir:rst shared memory.
    irrstSharedMem_ = (volatile uint32_t*)mappableAlloc(0x98);

    if (!irrstSharedMem_) {
        ret = -1;
        goto cleanup1;
    }

    if (R_FAILED(ret = svcMapMemoryBlock(irrstMemHandle_, (uint32_t)irrstSharedMem_, MEMPERM_READ, MEMPERM_DONTCARE)))
        goto cleanup2;

    return 0;

cleanup2:
    svcCloseHandle(irrstMemHandle_);

    if (irrstSharedMem_ != NULL) {
        mappableFree((void*) irrstSharedMem_);
        irrstSharedMem_ = NULL;
    }

cleanup1:
    svcCloseHandle(irrstHandle_);
cleanup0:
    AtomicDecrement(&irrstRefCount);
    return ret;
}


int irrstGetRefCount_() {
    return irrstRefCount;
}

void irrstExit_(void) {
    if (AtomicDecrement(&irrstRefCount))
        return;

    // Reset internal state

    svcCloseHandle(irrstEvent_);
    // Unmap ir:rst sharedmem and close handles.
    svcUnmapMemoryBlock(irrstMemHandle_, (uint32_t)irrstSharedMem_);

    if (envGetHandle("ir:rst") == 0)
        IRRST_Shutdown_();

    svcCloseHandle(irrstMemHandle_);
    svcCloseHandle(irrstHandle_);

    if (irrstSharedMem_ != NULL) {
        mappableFree((void*) irrstSharedMem_);
        irrstSharedMem_ = NULL;
    }
}