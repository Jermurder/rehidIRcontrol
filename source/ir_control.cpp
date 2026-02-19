#include <3ds.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "ir_control.hpp"

#define SHARED_MEM_SIZE 0x1000
#define MAX_BYTES 256
#define SYNC_BYTE 0xAA

Handle recvEvt;
u8 lastIndex = 0;

u32 *sharedMem;
bool g_receiveActive = false;

enum IRParseState
{
    IR_WAIT_SYNC,
    IR_WAIT_TYPE,
    IR_WAIT_VALUE
};

static IRParseState parseState = IR_WAIT_SYNC;
static u8 currentType = 0;

enum IRDataType : u8
{
    IR_TYPE_BTN1 = 0x01,
    IR_TYPE_BTN2 = 0x02,
    IR_TYPE_ANALOG_X = 0x03,
    IR_TYPE_ANALOG_Y = 0x04,
};

static u8 recv_btn1 = 0;
static u8 recv_btn2 = 0;
static u8 recv_analogX = 128;
static u8 recv_analogY = 128;

// FUNCTIONS NOT IMPLEMENTED IN LIBCTRU
// This returns the raw u8 state field checked by GetSendState.
Result IRU_GetTransferState(u32 *out)
{
    Handle iruHandle = iruGetServHandle();

    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0xF, 0, 0); // 0xF0000

    if (R_FAILED(ret = svcSendSyncRequest(iruHandle)))
        return ret;
    ret = (Result)cmdbuf[1];

    *out = cmdbuf[2];

    return ret;
}

/*If either of the I2C errors is triggered by one of the internal I2C operations failing,
most subsequent calls to IR services will fail with error 0xF9610C02 until the system is rebooted.
The system entering sleep mode will cancel any running transfer,
similar to calling IRU:WaitSendTransfer or IRU:WaitRecvTransfer.
Upon waking up, the error is set to 7 and the send or receive event is signalled,
depending on which transfer, if any, was running.
*/

Result IRU_GetErrorStatus(u32 *out)
{
    Handle iruHandle = iruGetServHandle();

    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x10, 0, 0); // 0x100000

    if (R_FAILED(ret = svcSendSyncRequest(iruHandle)))
        return ret;
    ret = (Result)cmdbuf[1];

    *out = cmdbuf[2];

    return ret;
}

Result IRU_GetRecvTransferCount(u32 *transfercount)
{
    Handle iruHandle = iruGetServHandle();

    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x7, 0, 0); // 0x70000

    if (R_FAILED(ret = svcSendSyncRequest(iruHandle)))
        return ret;
    ret = (Result)cmdbuf[1];

    *transfercount = cmdbuf[2];

    return ret;
}

static inline bool isServiceUsable(const char *name)
{
    bool r;
    return R_SUCCEEDED(srvIsServiceRegistered(&r, name)) && r;
}

u32 IR_GetButtons()
{
    u32 keys = 0;

    if (recv_btn1 & 0x01) keys |= KEY_A;
    if (recv_btn1 & 0x02) keys |= KEY_B;
    if (recv_btn1 & 0x04) keys |= KEY_X;
    if (recv_btn1 & 0x08) keys |= KEY_Y;

    if (recv_btn2 & 0x01) keys |= KEY_L;
    if (recv_btn2 & 0x02) keys |= KEY_R;
    if (recv_btn2 & 0x04) keys |= KEY_START;
    if (recv_btn2 & 0x08) keys |= KEY_SELECT;

    return keys;
}

int IR_GetStickX()
{
    return ((int)recv_analogX - 128) * 156;
}

int IR_GetStickY()
{
    return ((int)recv_analogY - 128) * 156;
}

bool IR_IsActive()
{
    return g_receiveActive;
}


void initIR()
{
    mcuHwcInit();

    sharedMem = (u32 *)memalign(0x1000, SHARED_MEM_SIZE);
    if (!sharedMem)
    {
        MCUHWC_SetPowerLedState(LED_BLINK_RED);
    }
    while (!isServiceUsable("ir:u"))
        svcSleepThread(1e+9);

    Result rc = iruInit(sharedMem, SHARED_MEM_SIZE);
    if (R_FAILED(rc))
    {
        MCUHWC_SetPowerLedState(LED_RED);
    }
    rc = IRU_SetBitRate(3);

    if (R_FAILED(rc))
    {
        MCUHWC_SetPowerLedState(LED_SLEEP_MODE);
    }
}

void startReceive()
{

    if (g_receiveActive)
        return;
    Result rc = IRU_GetRecvFinishedEvent(&recvEvt);
    if (R_FAILED(rc))
    {
    }

    rc = IRU_StartRecvTransfer(SHARED_MEM_SIZE, 1);
    if (R_FAILED(rc))
    {
    }

    g_receiveActive = true;
}

void IR_ParseByte(u8 byte)
{
    switch (parseState)
    {
    case IR_WAIT_SYNC:
        if (byte == SYNC_BYTE)
            parseState = IR_WAIT_TYPE;
        break;

    case IR_WAIT_TYPE:

        if (byte >= IR_TYPE_BTN1 && byte <= IR_TYPE_ANALOG_Y)
        {
            currentType = byte;
            parseState = IR_WAIT_VALUE;
        }
        else
        {
            parseState = IR_WAIT_SYNC;
        }
        break;

    case IR_WAIT_VALUE:

        switch (currentType)
        {
        case IR_TYPE_BTN1:
            recv_btn1 = byte;
            break;

        case IR_TYPE_BTN2:
            recv_btn2 = byte;
            break;

        case IR_TYPE_ANALOG_X:
            recv_analogX = byte;
            break;

        case IR_TYPE_ANALOG_Y:
            recv_analogY = byte;
            break;
        }

        parseState = IR_WAIT_SYNC;
        break;
    }
}

void pollReceive()
{
    if (!g_receiveActive)
        return;

    if (svcWaitSynchronization(recvEvt, 1) == 0)
    {
        svcClearEvent(recvEvt);

        u32 count = 0;
        IRU_GetRecvTransferCount(&count);

        u8 *data = (u8 *)sharedMem;

        for (u32 i = 0; i < count; i++)
        {
            IR_ParseByte(data[i]);
        }

        startReceive();
    }
}

void irExit()
{
    iruExit();
    free(sharedMem);
}