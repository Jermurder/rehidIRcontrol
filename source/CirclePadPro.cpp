#include <3ds.h>
#include <malloc.h>
#include <cstdlib>
#include "CirclePadPro.hpp"
#include "crc8.hpp"
#include "iruser.hpp"

Result CirclePadPro::Initialize() {
    Result ret = 0;

    if (R_FAILED(ret = irUserInit()))
        return ret;

    m_sharedmem = (CPPSharedMem *)aligned_alloc(0x1000, CPP_SHARED_MEM_SIZE);
    if (!m_sharedmem) {
        ret = -1;
        svcBreak(USERBREAK_ASSERT);
    }

    if (R_FAILED(ret = svcCreateMemoryBlock(&m_sharedmemhandle, (uint32_t)m_sharedmem, CPP_SHARED_MEM_SIZE, MEMPERM_READ, MEMPERM_READ))) {
        svcBreak(USERBREAK_ASSERT);
    }

    Handle *connectionevent = GetConnectionEvent();
    Handle *recvevent = GetRecvEvent();

    if (R_FAILED(ret = IRUSER_GetConnectionStatusEvent(connectionevent))) {
        svcBreak(USERBREAK_ASSERT);
    }

    if (R_FAILED(ret = IRUSER_GetReceiveEvent(recvevent))) {
        svcBreak(USERBREAK_ASSERT);
    }

    if (R_SUCCEEDED(ret = IRUSER_InitializeIrnopShared(m_sharedmemhandle, CPP_SHARED_MEM_SIZE, m_recvbufsize, CPP_RECV_PACKET_COUNT, m_sendbufsize, CPP_SEND_PACKET_COUNT, CPP_BAUD_RATE))) {
        return 0;
    }

    svcCloseHandle(*recvevent);
    svcCloseHandle(*connectionevent);
    svcCloseHandle(m_sharedmemhandle);
    free(m_sharedmem);
    irUserFinalize();

    return ret;
}

bool CirclePadPro::IsConnected() {
    return m_sharedmem->header.connectionstatus == IRUSER_ConnectionStatus::Connected;
}

void CirclePadPro::Sampling() {

    /* If we are not connected, we'll wait for a connection first */
    switch (m_sharedmem->header.connectionstatus) {
        case IRUSER_ConnectionStatus::Disconnected:
            IRUSER_RequireConnection(1);
            return;

        case IRUSER_ConnectionStatus::Connected:
            break;

        case IRUSER_ConnectionStatus::Disconnecting:
        case IRUSER_ConnectionStatus::Connecting:
        default:
            return;
    }

    ProcessRecvPackets();
    SendNextPacket();
}

void CirclePadPro::ProcessCalibrationDataPacket(void *packet) {
    CPPCalibrationResponse *responsePacket = (CPPCalibrationResponse *)packet;

    for (int i = 0; i < 4; i++) {
        CPPCalibrationResponseData candidate = responsePacket->candidates[i];
        if (crc8((void *)&candidate, 15) == candidate.crc8) {
            m_calibrationdata.xoff = candidate.xoff;
            m_calibrationdata.yoff = candidate.yoff;
            m_calibrationdata.xscale = candidate.xscale;
            m_calibrationdata.yscale = candidate.yscale;
            m_iscalibrationdone = true;
        }
    }
}

void CirclePadPro::ProcessRecvPackets() {
    if (m_expectedpacket == PacketType::None) {
        return;
    }

    size_t expectedsize = 0;

    if (m_expectedpacket == PacketType::CalibrationPacket)
        expectedsize = sizeof(CPPCalibrationResponse);
    else if (m_expectedpacket == PacketType::InputPacket)
        expectedsize = sizeof(CPPInputResponse);

    uint8_t response[expectedsize];

    int readlen = ReadPacket((void *)&response, expectedsize);
    if (readlen != expectedsize) { // || response[0] != m_expectedpacket) {
        m_lastreaderror = RecvPackageErrorType::ReadError;
    } else {
        m_lastreaderror = RecvPackageErrorType::None;
    }

    if (m_lastreaderror == RecvPackageErrorType::None) {
        switch (m_expectedpacket) {
            case PacketType::CalibrationPacket:
                ProcessCalibrationDataPacket(&response);
                break;

            case PacketType::InputPacket:
                ProcessInputPacket(&response);
                break;
        }
    }
}

void CirclePadPro::SendNextPacket() {
    PacketType nextPacketType = PacketType::None;

    /* Repeat the last package incase previous recv package had a read error */
    if (m_lastreaderror == RecvPackageErrorType::ReadError) {
        switch (m_expectedpacket) {
            case PacketType::CalibrationPacket:
                nextPacketType = PacketType::CalibrationRequestPacket;
                break;

            case PacketType::InputPacket:
                nextPacketType = PacketType::InputRequestPacket;
                break;
        }
    } else {

        switch (m_expectedpacket) {
            case PacketType::None:
                nextPacketType = PacketType::CalibrationRequestPacket;
                break;

            case PacketType::CalibrationPacket:
                nextPacketType = m_iscalibrationdone ? PacketType::CalibrationRequestPacket : PacketType::InputRequestPacket;
                break;

            case PacketType::InputPacket:
            default:
                nextPacketType = PacketType::InputRequestPacket;
                break;
        }
    }

    switch (nextPacketType) {
        case PacketType::CalibrationRequestPacket: {
            m_expectedpacket = PacketType::CalibrationPacket;
            CPPCalibrationRequest request {2, 100, 0, 0x40};

            request.offset = m_calibrationpacketcount == 0 ? 0 : m_calibrationpacketcount + 0x400;
            IRUSER_SendIrnop((void *)&request, sizeof(request));
            m_calibrationpacketcount++;
            break;
        }

        case PacketType::InputRequestPacket: {
            m_expectedpacket = PacketType::InputPacket;
            CPPInputRequest request {1, 0x20, 0x87};
            IRUSER_SendIrnop((void *)&request, sizeof(request));
        }
    }
}

int CirclePadPro::ReadPacket(void *out, size_t outlen) {
    // Check if there any packets to read at all
    if (m_sharedmem->bufferinfo.packetcount <= 0) {
        return 0;
    }

    CPPPacketInfo *info = &m_sharedmem->packetinfos[m_packetid];
    uint8_t *start = (uint8_t *)&m_sharedmem->packetinfos[CPP_RECV_PACKET_COUNT];
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

    int length = header[2] & 0x3f;
    if (header[2] & 0x40) {
        length = (length << 8) | header[3];
    } else {
        // Backtrack one byte.
        if (current == start) current = end;
        current--;
    }
    
    int lentoread = length <= outlen ? length : outlen;

    for (int i = 0; i < lentoread; i++) {
        *(uint8_t*)out++ = *current++;
        if (current == end) 
            current = start;
    }

    if (length <= outlen) 
        ClearPackets(1);

    return length;
}

Result CirclePadPro::ClearPackets(int packets) {
    Result ret = IRUSER_ReleaseSharedData(packets);
    if (R_FAILED(ret))
        return ret;

    m_packetid += packets;
    while (m_packetid >= CPP_RECV_PACKET_COUNT) {
        m_packetid -= CPP_RECV_PACKET_COUNT;
    }

    return 0;
}