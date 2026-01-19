#pragma once
#include <3ds.h>
#include "iruser.hpp"

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
    bool zl: 1;
    bool zr: 1;
    bool r: 1;
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
#pragma pack(pop)

struct CPPCalibrationResponse {
    uint8_t header;
    uint16_t offset;
    uint16_t size;
    CPPCalibrationResponseData candidates[4];
};

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

constexpr size_t CPP_SHARED_MEM_SIZE = 0x1000;
constexpr size_t CPP_RECV_PACKET_COUNT = 0xA0;
constexpr size_t CPP_SEND_PACKET_COUNT = 0x20;
constexpr size_t CPP_RECV_DATA_SIZE = 2000;
constexpr size_t CPP_SEND_DATA_SIZE = 0x200;
constexpr uint8_t CPP_BAUD_RATE = 4;

class CirclePadPro {
public:
    Result Initialize();

    bool IsConnected();

    void Sampling();

    Handle *GetConnectionEvent() {
        return &m_events[0];
    }

    Handle *GetRecvEvent() {
        return &m_events[1];
    }

private:
    void ProcessCalibrationDataPacket(void *packet);
    void ProcessInputPacket(void *packet);
    void ProcessRecvPackets();
    void SendNextPacket();
    int ReadPacket(void *out, size_t outlen);
    Result ClearPackets(int count);

    CPPSharedMem *m_sharedmem = nullptr;
    Handle m_sharedmemhandle;
    Handle m_connectionstatushandle;
    Handle m_events[3];

    static constexpr size_t m_recvbufsize = CPP_RECV_DATA_SIZE + CPP_RECV_PACKET_COUNT * 8;
    static constexpr size_t m_sendbufsize = CPP_SEND_DATA_SIZE + CPP_SEND_PACKET_COUNT * 8;

    uint8_t m_packetid = 0;

    bool m_iscalibrationdone = false;
    uint8_t m_calibrationpacketcount = 0;
    CPPCalibrationData m_calibrationdata {};

    PacketType m_expectedpacket = PacketType::None;
    RecvPackageErrorType m_lastreaderror = RecvPackageErrorType::None;
};