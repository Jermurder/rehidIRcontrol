#pragma once
#include <3ds.h>
#include "Remapper.hpp"
#include "rings/CPPRing.hpp"

class IR {

public:
    virtual Result Initialize() = 0;
    virtual void Sampling() = 0;
    virtual void ScanInput(Remapper *remapper) = 0;
    virtual CPPEntry GetInputs() = 0;
    virtual Handle *GetTimer() = 0;
    virtual uint8_t IsInitialized() = 0;
    virtual uint8_t IsEnteringSleep() = 0;
    virtual void SetEnteringSleep(uint8_t val) = 0;
    virtual void SetTimer() = 0;
    virtual bool IsConnected() = 0;
};
