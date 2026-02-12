#include "ipc.hpp"
#include "services/mcuhid.hpp"
#include <cstdio>

extern char buf[100];

namespace IPC {

void HandleHidCommands(Hid *hid) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    uint16_t cmdid = cmdbuf[0] >> 16;

    switch (cmdid) {
        case 1: { // TouchScreenCalibrateParam
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 2: { // TouchScreenFlushParam
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 3: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 4: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 5: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 6: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 7: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 8: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 9: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0xA: {
            //hid->TakeOverIRRSTIfRequired();
            //svcBreak(USERBREAK_ASSERT);
            //irCheckAndActivateIfRequired(tid);
            hid->RemapGenFileLoc();
            cmdbuf[0] = 0xA0047;
            cmdbuf[1] = 0;
            cmdbuf[2] = 0x14000000;
            cmdbuf[3] = *hid->GetSharedMemHandle();
            cmdbuf[4] = *hid->GetPad()->GetEvent();
            cmdbuf[5] = *hid->GetTouch()->GetEvent();
            cmdbuf[6] = *hid->GetAccelerometer()->GetEvent();
            cmdbuf[7] = *hid->GetGyroscope()->GetEvent();
            cmdbuf[8] = *hid->GetDebugPad()->GetEvent();
            break;
        }

        case 0xB: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0xC: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0xD: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0xE: {
            cmdbuf[0] = 0xE0200;
            cmdbuf[1] = 0;
            break;
        }

        case 0xF: {
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0x10: {
            cmdbuf[0] = 0x100100;
            cmdbuf[1] = 0;
            break;
        }

        case 0x11: { // EnableAccelerometer
            hid->GetAccelerometer()->EnableAndIncreementRef();
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0x12: { // DisableAccelerometer
            hid->GetAccelerometer()->DisableAndDecreementRef();
            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0x13: {
            hid->GetGyroscope()->IncreementHandleIndex();

            if (hid->GetGyroscope()->GetRefCount() > 0 && !hid->GetGyroscope()->IsEnabled()) {
                hid->GetGyroscope()->EnableSampling();
            }

            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0x14: {
            hid->GetGyroscope()->DecreementhandleIndex();

            if (hid->GetGyroscope()->GetRefCount() == 0) {
                hid->GetGyroscope()->m_issetupdone = false;
                hid->GetGyroscope()->DisableSampling();
            }

            cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
            cmdbuf[1] = 0;
            break;
        }

        case 0x15: {
            cmdbuf[0] = 0x150080;
            cmdbuf[1] = 0;
            cmdbuf[2] = 0x41660000;
            break;
        }

        case 0x16: {
            cmdbuf[0] = 0x160180;
            cmdbuf[1] = 0;
            hid->GetGyroscope()->GetCalibParam((GyroscopeCalibrateParam*)&cmdbuf[2]);
            break;
        }

        case 0x17: {
            uint8_t vol;
            cmdbuf[0] = 0x170080;
            cmdbuf[1] = mcuHidGetSoundVolume(&vol);
            cmdbuf[2] = vol;
            break;
        }

        case 0x18: {
            cmdbuf[0] = 0x180040;
            cmdbuf[1] = 0;
            break;
        }

        case 0x19: { // Special rehid IPC command
            cmdbuf[0] = 0x190040;
            cmdbuf[1] = 0;
            break;
        }

        default: {
            cmdbuf[1] = 0xD900182F;
            break;
        }
    }
}

void HandleMockIRUserCommands(Hid *hid) {
    uint32_t *cmdbuf = getThreadCommandBuffer();
    uint16_t cmdid = cmdbuf[0] >> 16;

    // sprintf(buf, "New cmid type : %X\n", cmdid);
    // svcOutputDebugString(buf, strlen(buf));

    IRUserMock *mock = hid->GetCPPO3DSObject()->GetMockIRUSERCommands();

    switch (cmdid) {
        case 0x1: // InitializeIrnop
        case 0x18: { // InitializeIrnopShared
            cmdbuf[0] = 0x10040;
            cmdbuf[1] = mock->InitializeIrNopShared(cmdbuf[1], cmdbuf[2], cmdbuf[3], cmdbuf[4], cmdbuf[5], cmdbuf[6], cmdbuf[8]);
            break;
        }

        case 0x2: { // Finalize
            cmdbuf[0] = 0x20040;
            cmdbuf[1] = mock->IrNopFinalize();
            break;
        }

        case 0x3: { // ClearRecieveBuffer
            cmdbuf[0] = 0x30040;
            cmdbuf[1] = mock->ClearReceiveBuffer();
            break;
        }

        case 0x4: { // ClearSendBuffer
            cmdbuf[0] = 0x40040;
            cmdbuf[1] = 0;
            break;
        }

        case 0x6: { // RequireConnection
            cmdbuf[0] = 0x60040;
            cmdbuf[1] = mock->RequireConnection((uint8_t)(cmdbuf[1] & 0xFF));
            break;
        }

        case 0x9: { // Disconnect
            cmdbuf[0] = 0x90040;
            cmdbuf[1] = mock->Disconnect();
            break;
        }

        case 0xA: { // GetRecvEvent
            Handle event;

            cmdbuf[0] = 0xA0042;
            cmdbuf[1] = mock->GetRecvEvent(&event);
            cmdbuf[2] = IPC_Desc_SharedHandles(1);
            cmdbuf[3] = event;
            break;
        }

        case 0xB: { // GetSendEvent
            Handle event;

            cmdbuf[0] = 0xB0042;
            cmdbuf[1] = mock->GetSendEvent(&event);
            cmdbuf[2] = IPC_Desc_SharedHandles(1);
            cmdbuf[3] = event;
            break;
        }

        case 0xC: { // GetConnectionStatusEvent
            Handle event;

            cmdbuf[0] = 0xC0042;
            cmdbuf[1] = mock->GetConnectionStatusEvent(&event);
            cmdbuf[2] = IPC_Desc_SharedHandles(1);
            cmdbuf[3] = event;
            break;
        }

        case 0xD: { // SendIrnop
            cmdbuf[0] = 0xD0040;
            cmdbuf[1] = mock->SendIrnop((uint8_t *)cmdbuf[3], cmdbuf[2] >> 14);
            break;
        }

        case 0xE: { // SendIrnopLarge
            cmdbuf[0] = 0xE0042;
            cmdbuf[1] = mock->SendIrnop((uint8_t *)cmdbuf[3], cmdbuf[2] >> 4);
            cmdbuf[2] = (16 * (cmdbuf[2] >> 4)) | 12;
            break;
        }

        case 0x19: { // ReleaseRecievedData
            cmdbuf[0] = 0x190040;
            cmdbuf[1] = mock->ReleaseRecievedData(cmdbuf[1]);
            break;
        }

        default: {
            cmdbuf[0] = 0x40;
            cmdbuf[1] = 0xD900182F;
            break;
        }
    }
}

} // End of namespace IPC