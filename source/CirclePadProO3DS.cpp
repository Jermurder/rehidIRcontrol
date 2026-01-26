#include <3ds.h>
#include <malloc.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "crc8.hpp"
#include "iruser.hpp"
#include "CirclePadProO3DS.hpp"

extern void _putchar(char character);

#ifdef CTR_ALIGN
static uint8_t CTR_ALIGN(8) irthreadstack[0x1000];
#else
static uint8_t ALIGN(8) irthreadstack[0x1000];
#endif

static void cppthread(void *param);

Result CPPO3DS::Initialize() {
    Result ret = 0;
    m_packetid = 0;

    m_sharedmem = (CPPSharedMem *)aligned_alloc(0x1000, CPP_SHARED_MEM_SIZE);
    if (m_sharedmem == NULL) {
        ret = -1;
        return ret;
    }

    ret = irUserInit();
    if (R_FAILED(ret)) 
        goto cleanup1;

    ret = svcCreateMemoryBlock(&m_sharedmemhandle, (uint32_t)m_sharedmem, CPP_SHARED_MEM_SIZE, MEMPERM_READ, MEMPERM_READWRITE);
    if (R_FAILED(ret))
        goto cleanup2;

    ret = svcCreateEvent(GetExitEvent(), RESET_ONESHOT);
    if (R_FAILED(ret)) 
        goto cleanup3;

    ret = svcCreateEvent(GetSleepEvent(), RESET_ONESHOT);
    if (R_FAILED(ret)) 
        goto cleanup4;

    ret = MyThread_Create(&m_thread, cppthread, this, irthreadstack, 0x1000, 0x20, -2);

    if (R_FAILED(ret)) {
        ret = -1;
        goto cleanup5;
    }

    LightLock_Init(GetSleepLock());

    return 0;

    cleanup5:
    svcCloseHandle(*GetSleepEvent());
    cleanup4:
    svcCloseHandle(*GetExitEvent());
    cleanup3:
    svcCloseHandle(m_sharedmemhandle);
    cleanup2:
    irUserFinalize();
    cleanup1:
    free(m_sharedmem);
    m_sharedmem = nullptr;
    return ret;
}

void CPPO3DS::SetTimer() {
    svcCreateTimer(&m_timer, RESET_ONESHOT);

    if (R_FAILED(svcSetTimer(m_timer, 8000000LL, 0LL)))
        svcBreak(USERBREAK_ASSERT);
}

void CPPO3DS::Sampling() {
    svcSetTimer(m_timer, 8000000LL, 0LL);

    CPPEntry entry;
    entry.pressedpadstate = (m_latestkeys ^ m_oldkeys) & ~m_oldkeys;
    entry.releasedpadstate = (m_latestkeys ^ m_oldkeys) & m_oldkeys;
    entry.currpadstate = m_latestkeys;
    m_ring.WriteToRing(&entry);

    m_oldkeys = m_latestkeys;
}

void CPPO3DS::ScanInput(Remapper *remapper) {
    m_keysstate = GetLatestInputFromRing();
}

static uint32_t CheckSectionUpdateTime(uint32_t id, CPPRing *ring) {
    int64_t tick0 = 0, tick1 = 0;

    if (id == 0) {
        tick0 = ring->GetTickCount();
        tick1 = ring->GetOldTickCount();

        if (tick0 == tick1 || tick0 < 0 || tick1 < 0)
            return 1;
    }

    return 0;
}

uint32_t CPPO3DS::GetLatestInputFromRing() {
    uint32_t id = 0;
    uint32_t latest = 0;

    id = m_ring.GetIndex(); //PAD / circle-pad

    if (id > 7)
        id = 0;

    if (CheckSectionUpdateTime(id, &m_ring) == 1)
        return 0;

    latest = m_ring.GetLatest(id);

    return latest;
}

void CPPO3DS::Disconnect() {
    svcCloseHandle(*GetConnectionEvent());
    svcCloseHandle(*GetRecvEvent());

    m_isconnected = false;

    IRUSER_Disconnect();
    IRUSER_ClearReceiveBuffer();
    IRUSER_ClearSendBuffer();
    IRUSER_FinalizeIrnop();

    //batteryLevel = 0;

    m_latestkeys = 0;
    m_packetid = 0;

    // cPos.dx = 0;
    // cPos.dy = 0;
}

int CPPO3DS::WaitForConnection() {
    int32_t handleidx = 0;

    Handle *connectionevent = GetConnectionEvent();
    Handle *recvevent = GetRecvEvent();

    Handle events[3] {};
    events[1] = *GetExitEvent();
    events[2] = *GetSleepEvent();

    Result ret = 0;

    while (true && !m_sleep) {
        // If Lock succeeds, then we are not sleeping
        LightLock_Lock(GetSleepLock());

        // I have no idea why this can't be out side the loop
        // Having it outside the loop doesn't work
        IRUSER_InitializeIrnopShared(m_sharedmemhandle, \
                CPP_SHARED_MEM_SIZE, m_recvbufsize, CPP_RECV_PACKET_COUNT, \
                m_sendbufsize, CPP_SEND_PACKET_COUNT, CPP_BAUD_RATE);

        IRUSER_GetConnectionStatusEvent(connectionevent);
        IRUSER_GetReceiveEvent(recvevent);

        events[0] = *connectionevent;

        // Try four times in quick succession, then wait a second.
        for (int i = 0; i < 4; i++) {
            IRUSER_RequireConnection(1);
            ret = svcWaitSynchronizationN(&handleidx, events, 3, false, 14 * MILLIS);
            if (R_DESCRIPTION(ret) != RD_TIMEOUT) {
                if (handleidx == 0) {
                    LightLock_Unlock(GetSleepLock());
                    return 0;
                }

                else if (handleidx == 1) {
                    LightLock_Unlock(GetSleepLock());
                    return -1;
                }

                else if (handleidx == 2) {
                    LightLock_Unlock(GetSleepLock());
                    return -2;
                }

                break;
            }

            IRUSER_Disconnect();
        }

        LightLock_Unlock(GetSleepLock());
        Disconnect();

        events[0] = *GetSleepEvent();

        LightLock_Lock(GetSleepLock());
        ret = svcWaitSynchronizationN(&handleidx, events, 2, false, 1000 * MILLIS);
        if (R_DESCRIPTION(ret) != RD_TIMEOUT) {
            if (handleidx == 1) {
                LightLock_Unlock(GetSleepLock());
                return -1;
            }

            else if (handleidx == 0) {
                LightLock_Unlock(GetSleepLock());
                return -2;
            }
        }

        LightLock_Unlock(GetSleepLock());
    }

    LightLock_Unlock(GetSleepLock());
    return -1;
}

Result CPPO3DS::SendReceive(const void *request, size_t requestsize, void *response, int responsesize, Handle *events, int64_t timeout, uint8_t expectedhead) {
    Result subres, res;
    for (int i = 0; i < 4; i++) {
        IRUSER_SendIrnop(request, requestsize);

        int32_t handleidx = 0;
        subres = svcWaitSynchronizationN(&handleidx, events, 3, false, timeout);
        if (R_DESCRIPTION(subres) == RD_TIMEOUT || handleidx == 1)  {
            // Timeout.
            res = SAR_TIMEOUT;
            continue;
        }

        if (handleidx == 2) {
            // Exit.
            res = SAR_EXIT;
            break;
        }

        subres = ReadPacket(response, responsesize);
        if (subres != responsesize || *(uint8_t*)response != expectedhead) {
            // Read error.
            res = SAR_READERROR;
            continue;
        }

        return 0;
    }

    return res;
}

static bool CheckCalibrationData(CPPCalibrationData *out, CPPCalibrationResponseData *candidate) {
    if (crc8(candidate, 15) == candidate->crc8) {
        out->xoff = candidate->xoff;
        out->yoff = candidate->yoff;
        out->xscale = candidate->xscale;
        out->yscale = candidate->yscale;
        return true;
    }

    return false;
}

int CPPO3DS::GetCalibrationData() {
    bool found = false;
    CPPCalibrationRequest request {2, 100, 0, 0x40};
    CPPCalibrationResponse response;

    Handle events[3] {*GetRecvEvent(), *GetSleepEvent(), *GetExitEvent()};

    Result ret = SendReceive(&request, sizeof(request), &response, sizeof(response), events, 20 * MILLIS, 0x11);
    if (ret == SAR_EXIT) {
        return -1;
    }

    if (ret == SAR_READERROR || ret == SAR_TIMEOUT) {
        Disconnect();
        return -2;
    }

    for (int i = 0; i < 4; i++) {
        if (CheckCalibrationData(&m_calibrationdata, &response.candidates[i])) {
            found = true;
            return 0;
        }
    }

    if (!found) {
        for (int i = 0; i < 0x40; i++) {
            request.offset = 0x400 + i;
            ret = SendReceive(&request, sizeof(request), &response, sizeof(response), events, 20 * MILLIS, 0x11);
            if (ret == SAR_EXIT) {
                return -1;
            }

            if (ret == SAR_READERROR || ret == SAR_TIMEOUT) {
                Disconnect();
                return -2;
            }

            for (int j = 0; j < 4; j++) {
                if (CheckCalibrationData(&m_calibrationdata, &response.candidates[j])) {
                    found = true;
                    return 0;
                }
            }
        }
    }

    if (!found)
        Disconnect();

    return -2;
}

int CPPO3DS::GetInputPackets() {
    CPPInputRequest request {0x01, 8, 0x87};
    Handle events[3] {*GetRecvEvent(), *GetSleepEvent(), *GetExitEvent()};

    while (m_sharedmem->header.connectionstatus == IRUSER_ConnectionStatus::Connected && !m_sleep) {
        CPPInputResponse response {};

        Result ret = SendReceive(&request, sizeof(request), &response, sizeof(response), events, 20 * MILLIS, 0x10);
        if (ret == SAR_EXIT) {
            return -1;
        }

        if (ret == SAR_TIMEOUT) {
            Disconnect();
            return -2;
        }

        if (ret == SAR_READERROR || response.header != 0x10) {
            continue; // Ignore read errors.
        }

        //cPos.dx = (s16)((float)(s16)(response.x - m_calibrationdata.xoff) * m_calibrationdata.xscale) / 8;
        //cPos.dy = (s16)((float)(s16)(response.y - m_calibrationdata.yoff) * m_calibrationdata.yscale) / 8;

        uint32_t keys = 0;
        if (!response.rup) keys |= KEY_R;
        if (!response.zlup) keys |= KEY_ZL;
        if (!response.zrup) keys |= KEY_ZR;
        m_latestkeys = keys;

        //batteryLevel = input_response.battery_level;
    }

    return 0;
}

static void cppthread(void *param) {
    CPPO3DS *cpp = (CPPO3DS *)param;

    while (true) {

        // Wait for CPP connection.
        int result = cpp->WaitForConnection();
        if (result == -1)
            break;
        else if (result == -2) {
            LightLock_Unlock(cpp->GetSleepLock());
            continue;
        }


        // If Lock succeeds, then we are not sleeping
        LightLock_Lock(cpp->GetSleepLock());

        // Retrieve calibration data.
        result = cpp->GetCalibrationData();
        if (result == -1)
            break;
        else if (result == -2) {
            LightLock_Unlock(cpp->GetSleepLock());
            continue;
        }

        // Mark connection as connected
        cpp->SetConnected(true);

        // Process Input packets while connection is connected
        result = cpp->GetInputPackets();
        if (result == -1)
            break;
        else if (result == -2) {
            LightLock_Unlock(cpp->GetSleepLock());
            continue;
        }

        LightLock_Unlock(cpp->GetSleepLock());
    }

    cpp->Disconnect();

    svcCloseHandle(*cpp->GetSharedMemHandle());
    irUserFinalize();

    CPPSharedMem *mem = cpp->GetSharedMem();
    free(mem);
    mem = nullptr;
}

int CPPO3DS::ReadPacket(void *out, size_t outlen) {
    // Check if there any packets to read at all
    if (m_sharedmem->bufferinfo.packetcount <= 0) {
        return 0;
    }

    CPPPacketInfo *info = &m_sharedmem->packetinfos[m_packetid];
    uint8_t *start = ((uint8_t *)&m_sharedmem->packetinfos[CPP_RECV_PACKET_COUNT]);
    uint8_t *end = start + m_recvbufsize;

    uint8_t *current = start + info->offset;

    if (crc8_loop(start, m_recvbufsize, current, info->size)) {
        // Invalid Packet
        ClearPackets(1);
        return 0;
    }

    uint8_t header[4];
    for (int i = 0; i < 4; i++) {
        header[i] = *current++;
        if (current == end)
            current = start;
    }

    size_t length = header[2] & 0x3f;
    if (header[2] & 0x40) {
        length = (length << 8) | header[3];
    } else {
        // Backtrack one byte.
        if (current == start) current = end;
        current--;
    }
    
    size_t lentoread = length <= outlen ? length : outlen;

    for (size_t i = 0; i < lentoread; i++) {
        *(uint8_t*)out++ = *current++;
        if (current == end) 
            current = start;
    }

    if (length <= outlen) 
        ClearPackets(1);

    return length;
}

Result CPPO3DS::ClearPackets(int packets) {
    Result ret = IRUSER_ReleaseSharedData(packets);
    if (R_FAILED(ret))
        return ret;

    m_packetid += packets;
    while (m_packetid >= CPP_RECV_PACKET_COUNT) {
        m_packetid -= CPP_RECV_PACKET_COUNT;
    }

    return 0;
}