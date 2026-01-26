#pragma once
#include <3ds.h>
#include "Touch.hpp"
#include "Pad.hpp"
#include "Accelerometer.hpp"
#include "Gyroscope.hpp"
#include "DebugPad.hpp"
#include "ir.hpp"
#include "CirclePadProO3DS.hpp"
#include "CirclePadProN3DS.hpp"

extern "C"
{
#include "mythread.h"
#include "codec.h"
}

class Hid {
public:

    Hid() {
        int64_t out = 0;
        uint8_t isn3ds = svcGetSystemInfo(&out, 0x10001, 0) == 0;

        if (isn3ds)
            m_ir = &m_cppn3ds;
        else
            m_ir = &m_cppo3ds;
    }

    void CreateAndMapMemoryBlock();
    void CreateRingsOnSharedmemoryBlock();
    void StartThreadsForSampling();
    void InitializePad();
    void InitializeAccelerometer();
    void InitializeGyroscope();
    void InitializeDebugPad();
    void InitializeIR();
    void EnteringSleepMode();
    void ExitingSleepMode();
    void IsShellOpened(bool opened) {
        m_shellisopen = opened;
    };

    Pad *GetPad() {
        return &m_pad;
    };

    Touch *GetTouch() {
        return &m_touch;
    };

    Accelerometer *GetAccelerometer() {
        return &m_accel;
    };

    Gyroscope *GetGyroscope() {
        return &m_gyro;
    };

    DebugPad *GetDebugPad() {
        return &m_debugpad;
    };

    IR *GetIR() {
        return m_ir;
    }

    Handle *GetSharedMemHandle() {
        return &m_sharedmemhandle;
    };

    uint8_t *ExitThread() {
        return &m_exitthread;
    };

    LightLock *GetSleepLock() {
        return &m_sleeplock;
    };

    Remapper *GetRemapperObject() {
        return &m_remapper;
    };

    void RemapGenFileLoc();
    void CheckIfIRPatchExists();
    Handle dummyhandles[4] = {0};
private:
    Handle m_sharedmemhandle;
    void *m_addr = nullptr;
    PadRing *m_padring = nullptr;
    TouchRing *m_touchring = nullptr;
    AccelerometerRing *m_accelring = nullptr;
    GyroscopeRing *m_gyroring = nullptr;
    DebugPadRing *m_debugpadring = nullptr;
    Pad m_pad;
    Touch m_touch;
    Accelerometer m_accel;
    Gyroscope m_gyro;
    DebugPad m_debugpad;
    IR *m_ir;
    CPPO3DS m_cppo3ds;
    CPPN3DS m_cppn3ds;
    bool m_shellisopen = true;
    MyThread m_samplingthread;
    bool m_samplingthreadstarted = false;
    uint8_t m_exitthread = 0;
    LightLock m_sleeplock;
    Remapper m_remapper;
};