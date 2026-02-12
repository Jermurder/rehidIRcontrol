#include <3ds.h>
#include "csvc.h"
#include "memory.h"
#include "process_patches.h"

static inline uint32_t makeArmBranch(const void *src, const void *dst, bool link) { // the macros for those are ugly and buggy
    uint32_t instrBase = link ? 0xEB000000 : 0xEA000000;
    uint32_t off = (uint32_t)((const u8 *)dst - ((const u8 *)src + 8)); // the PC is always two instructions ahead of the one being executed

    return instrBase | ((off >> 2) & 0xFFFFFF);
}


Result irUserCustomIPCHandler() {

}

static Result DoUndoIRPatch(Handle processhandle) {
    static uint32_t ironwakeupcode[] = {
        0xE3500001, // CMP R0, #1
        0x13A00001, // MOVNE R0, #1
        0x03A00000, // MOVEQ R0, #0
        0x00000000, // bl 
        0x00000000, // NOP => bl addrof(irsleepjumpcode)
    };

    static uint32_t irsleepjumpcode[] = {
        0xE59F1008, // LDR R1, =irStruct
        0xE3A00000, // MOV R0, #0
    };

    static uint32_t serviceipc[] = {
        // iruipchandler // -12
        // iruseripchandler => irusercustomipchandler -8
        // irrstipchandler // -4
        0x753A7269, // "ir:u"
    };

    int64_t startaddress = 0, textsize = 0, rodatasize = 0, datasize = 0;
    uint32_t totalsize = 0;
    Result res = 0;

    svcGetProcessInfo(&textsize, processhandle, 0x10002);
    svcGetProcessInfo(&rodatasize, processhandle, 0x10003);
    svcGetProcessInfo(&datasize, processhandle, 0x10004);

    totalsize = (uint32_t)(textsize + rodatasize + datasize);

    res = svcGetProcessInfo(&startaddress, processhandle, 0x10005);
    if (R_FAILED(res)) {
        *(u32*)res = 0xA56;
    }

    res = svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x01000000, processhandle, (uint32_t) startaddress, totalsize, 0);

    if (R_SUCCEEDED(res)) {

        // Sleep Mode patch
        {
            uint32_t *jumpcodeoff = (uint32_t *)memsearch((uint8_t *)0x01000000, &irsleepjumpcode, totalsize, sizeof(irsleepjumpcode));
            if (jumpcodeoff == NULL) { // pattern not found
                svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x00100000, totalsize);
                *(u32*)0xAF0 = 0xFFFFFFFF;
                return -1;
            }

            uint32_t *wakeupcodeoff = (uint32_t *)memsearch((uint8_t *)0x01000000, &ironwakeupcode, totalsize, sizeof(ironwakeupcode) - 8);
            if (wakeupcodeoff == NULL) { // pattern not found
                svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x00100000, totalsize);
                *(u32*)0xAFC = 0xFFFFFFFF;
                return -1;
            }

            *(uint32_t *)(wakeupcodeoff + 5) = makeArmBranch((void *)((wakeupcodeoff + 5) - (0x1000000) + 0x100000) , (void *)(jumpcodeoff - (0x1000000) + 0x100000) , true);
        }

        // patch ipc tables
        // {
        //     uint32_t *ipctable = (uint32_t*)memsearch(((uint8_t *)0x01000000, &serviceipc, totalsize, sizeof(serviceipc)))

        // }

        svcInvalidateEntireInstructionCache();
        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x01000000, totalsize);
    }

    return res;
}

static Result IRGetprocesshandle(Handle* processhandle) {
    return OpenProcessByName("ir", processhandle);
}

static Handle IRLockThreads(Handle processhandle) {
    if (processhandle || R_SUCCEEDED(IRGetprocesshandle(&processhandle)))
        // Lock all ir threads
        svcControlProcess(processhandle, PROCESSOP_SCHEDULE_THREADS, 1, 0);
    return processhandle;
}

static Handle IRUnlockThreads(Handle processhandle) {
    if (processhandle || R_SUCCEEDED(IRGetprocesshandle(&processhandle)))
        // Unlock all ir threads
        svcControlProcess(processhandle, PROCESSOP_SCHEDULE_THREADS, 0, 0);
    return processhandle;
}

void DoPatchesForIR() {
    Handle irhandle = IRLockThreads(0);
    if (!irhandle) {
        *(u32*)0xAF0 = 0xFFFFFFFF;
        svcBreak(USERBREAK_ASSERT);
    }

    Result res = DoUndoIRPatch(irhandle);
    if (R_FAILED(res)) {
        *(u32*)res = 0xA64;
        svcBreak(USERBREAK_ASSERT);
    }

    IRUnlockThreads(irhandle);
    svcCloseHandle(irhandle);
}