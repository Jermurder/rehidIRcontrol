#pragma once
#include <3ds.h>
#include "ir.hpp"
#include "Remapper.hpp"
#include "CPPRing.hpp"


/* Samples controls handled by the IR module, i.e, CPAD Pro/Nub */
class CPPN3DS : public IR {
public:
    Result Initialize() override;
    void Sampling() override;
    void ScanInput(Remapper *remapper) override;

    CPPEntry GetInputs() override {
        return m_cppstate;
    }

    Handle *GetTimer() override {
        return &m_timer;
    }

    uint8_t IsInitialized() override {
        return m_initialized;
    }

    uint8_t IsEnteringSleep() override {
        return m_enteringsleep;
    }

    void SetEnteringSleep(uint8_t val) override {
        m_enteringsleep = val;
    }

    void SetTimer() override;

    bool IsConnected() override {
        return true; // Always connected on n3ds
    }

private:
    CPPEntry GetLatestInputFromRing();

    Handle m_timer;
    uint32_t *m_latestkeyspa = nullptr;
    uint32_t *m_statepa = nullptr;
    uint32_t m_latestkeys = 0;
    CPPEntry m_cppstate {};

    uint8_t m_initialized = 0;
    uint8_t m_enteringsleep = 0;
    uint8_t m_overridecpadpro = 0;
    CPPRing m_ring {};
};