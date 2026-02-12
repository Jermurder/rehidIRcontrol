#pragma once
#include <3ds.h>

enum class IRUSER_ConnectionStatus: uint8_t {
    Disconnected = 0,
    Connecting,
    Connected,
    Disconnecting,
};

Result irUserInit();
void irUserFinalize();
Result IRUSER_FinalizeIrnop(void);
Result IRUSER_ClearReceiveBuffer(void);
Result IRUSER_ClearSendBuffer(void);
Result IRUSER_RequireConnection(uint8_t deviceId);
Result IRUSER_Disconnect(void);
Result IRUSER_GetReceiveEvent(Handle *out);
Result IRUSER_GetConnectionStatusEvent(Handle *out);
Result IRUSER_SendIrnop(const void *buffer, uint32_t size);
Result IRUSER_InitializeIrnopShared(Handle shhandle, size_t sharedbufsize, size_t recvbufsize,
    size_t recvbufpacketcount, size_t sendbufsize, size_t sendbufpacketcount,
    uint8_t baudrate);
Result IRUSER_ReleaseSharedData(uint32_t count);
