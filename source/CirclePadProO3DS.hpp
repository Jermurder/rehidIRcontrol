#pragma once
#include <3ds.h>
#include "ir.hpp"
#include "iruser.hpp"

extern "C" {
    #include "mythread.h"
}

struct CPPSharedMemHeader {
    uint32_t recieveresult;
    uint32_t sendresult;
    IRUSER_ConnectionStatus connectionstatus;
    uint8_t tryingtoconnectstatus;
    uint8_t connectionrole;
    uint8_t machineid;
    uint8_t connected;
    uint8_t networkid;
    uint8_t initialized;
};

struct CPPBufferInfo {
    uint32_t beginindex;
    uint32_t endindex;
    uint32_t packetcount;
    uint32_t unknown;
};

struct CPPPacketInfo {
    uint32_t offset;
    uint32_t size;
};

struct CPPSharedMem {
    CPPSharedMemHeader header;
    CPPBufferInfo bufferinfo;
    CPPPacketInfo packetinfos[];
};

struct CPPInputRequest {
    uint8_t header;
    uint8_t responsetime;
    uint8_t unknown;
};

#pragma pack(push, 1)
struct CPPInputResponse {
    uint8_t header;
    uint16_t x: 12;
    uint16_t y: 12;
    uint8_t batterylevel: 5;
    bool zlup: 1;
    bool zrup: 1;
    bool rup: 1;
    uint8_t unknown;
};
#pragma pack(pop)

struct CPPCalibrationRequest {
    uint8_t header;
    uint8_t responsetime;
    uint16_t offset;
    uint16_t size;
};

struct CPPCalibrationData {
    uint16_t xoff;
    uint16_t yoff;
    float xscale;
    float yscale;
};

#pragma pack(push, 1)
struct CPPCalibrationResponseData {
    uint8_t pad1;
    uint16_t xoff: 12;
    uint16_t yoff: 12;
    float xscale;
    float yscale;
    uint8_t pad2[3];
    uint8_t crc8;
};

struct CPPCalibrationResponse {
    uint8_t header;
    uint16_t offset;
    uint16_t size;
    CPPCalibrationResponseData candidates[4];
};
#pragma pack(pop)

enum class PacketType : uint8_t {
    InputRequestPacket = 1,
    CalibrationRequestPacket = 2,
    InputPacket = 0x10,
    CalibrationPacket = 0x11,
    None = 0xFF,
};

enum class RecvPackageErrorType: uint8_t {
    None = 0,
    ReadError = 1,
};

enum {
    SAR_EXIT = -1,
    SAR_READERROR = -2,
    SAR_TIMEOUT = -3,
};

constexpr size_t CPP_SHARED_MEM_SIZE = 0x1000;
constexpr size_t CPP_RECV_PACKET_COUNT = 0xA0;
constexpr size_t CPP_SEND_PACKET_COUNT = 0x20;
constexpr size_t CPP_RECV_DATA_SIZE = 2000;
constexpr size_t CPP_SEND_DATA_SIZE = 0x200;
constexpr uint8_t CPP_BAUD_RATE = 4;
constexpr uint64_t MILLIS = 1000000;

extern "C" u32 cppKeysDown(void);

class CPPO3DS : public IR {
public:
    Result Initialize() override;

    uint8_t IsInitialized() override {
        return m_initialized;
    }

    void Sampling() override;

    void SetTimer() override;

    Handle *GetTimer() override {
        return &m_timer;
    }

    void ScanInput(Remapper *remapper) override;

    virtual CPPEntry GetInputs() override {
        return m_cppstate;
    }

    virtual uint8_t IsEnteringSleep() override {
        return 0;
    }

    virtual void SetEnteringSleep(uint8_t val) override {
        m_sleep = val;
        if (m_sleep) {
            svcSignalEvent(*GetExitEvent());
            MyThread_Join(&m_thread, 1000 * MILLIS);
        } else {
            m_ring.Reset();
            m_latestkeys = 0;
            m_cppstate = {};

            MCUHWC_SetPowerLedState(LED_OFF);
            CreateThread(); 
        }

        return;
    }

    void SetConnected(bool connected) {
        m_isconnected = connected;
    }

    bool IsConnected() override {
        return m_isconnected;
    }

    void Disconnect();

    int WaitForConnection();
    int GetCalibrationData();
    int GetInputPackets();

    Handle *GetConnectionEvent() {
        return &m_events[0];
    }

    Handle *GetRecvEvent() {
        return &m_events[1];
    }

    Handle *GetSharedMemHandle() {
        return &m_sharedmemhandle;
    }

    Handle *GetExitEvent() {
        return &m_events[2];
    }

    CPPSharedMem *GetSharedMem() {
        return m_sharedmem;
    }

    LightLock *GetSleepLock() {
        return &m_sleeplock;
    }

    Handle *GetSleepEvent() {
        return &m_events[3];
    }

    bool GetSleep() {
        return m_sleep;
    }

    Result CreateThread();

    InfoLedPattern m_pattern {};

private:
    Result SendReceive(const void *request, size_t requestsize, void *response, int responsesize, Handle *events, int64_t timeout, uint8_t expectedhead);
    int ReadPacket(void *out, size_t outlen);
    Result ClearPackets(int count);
    CPPEntry GetLatestInputFromRing();

    uint8_t m_initialized = 0;

    CPPSharedMem *m_sharedmem = nullptr;
    Handle m_sharedmemhandle;
    Handle m_connectionstatushandle;
    Handle m_events[4];
    Handle m_timer;

    LightLock m_sleeplock;

    MyThread m_thread;

    static constexpr size_t m_recvbufsize = CPP_RECV_DATA_SIZE + CPP_RECV_PACKET_COUNT * 8;
    static constexpr size_t m_sendbufsize = CPP_SEND_DATA_SIZE + CPP_SEND_PACKET_COUNT * 8;

    uint8_t m_packetid = 0;

    bool m_isconnected = false;
    bool m_sleep = false;

    uint8_t m_calibrationpacketcount = 0;
    CPPCalibrationData m_calibrationdata {};

    uint32_t m_latestkeys = 0;
    CirclePadEntry m_latestentry {};

    uint32_t m_oldkeys = 0;
    CPPEntry m_cppstate {};

    PacketType m_expectedpacket = PacketType::None;
    RecvPackageErrorType m_lastreaderror = RecvPackageErrorType::None;

    CPPRing m_ring {};
};