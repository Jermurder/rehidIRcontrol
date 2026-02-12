#pragma once
#include <3ds.h>
#include "services/iruser.hpp"

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
static_assert(sizeof(CPPSharedMemHeader) == 0x10, "Size of CPPSharedMemHeader is invalid");

struct CPPBufferInfo {
    uint32_t beginindex;
    uint32_t endindex;
    uint32_t packetcount;
    uint32_t unknown;
};
static_assert(sizeof(CPPBufferInfo) == 0x10, "Size of CPPBufferInfo is invalid");

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
static_assert(sizeof(CPPCalibrationResponseData) == 0x10, "Size of CPPCalibrationResponseData is invalid");

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
