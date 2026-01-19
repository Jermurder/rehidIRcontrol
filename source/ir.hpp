#pragma once
#include <3ds.h>
#include "Remapper.hpp"
#include "CPPRing.hpp"

/* Samples controls handled by the IR module, i.e, CPAD Pro/Nub */
class IR {
public:
    Result Initialize();
    void Sampling();
    void ScanInput(Remapper *remapper);

    uint32_t GetInputs() {
        return m_latestkeys;
    }

    Handle *GetTimer() {
        return &m_timer;
    }

    uint8_t IsInitialized() {
        return m_initialized;
    }

    uint8_t IsEnteringSleep() {
        return m_enteringsleep;
    }

    void SetEnteringSleep(uint8_t val) {
        m_enteringsleep = val;
    }

    void SetTimer();

private:
    uint32_t GetLatestInputFromRing();

    Handle m_timer;
    uint32_t *m_latestkeyspa = nullptr;
    uint32_t *m_statepa = nullptr;
    uint32_t m_latestkeys = 0;
    uint32_t m_keysstate = 0;

    uint8_t m_initialized = 0;
    uint8_t m_enteringsleep = 0;
    uint8_t m_overridecpadpro = 0;
    CPPRing m_ring {};
};