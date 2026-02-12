#include <3ds.h>
#include "rings/PadRing.hpp"
#include "CirclePad.hpp"
#include "Remapper.hpp"
#include "slider.hpp"
#include "ir.hpp"
#define IOHIDPAD *(vu16*)0x1EC46000

class Pad {
public:
    void Initialize();
    void SetPadRing(PadRing *ring) {
        m_ring = ring;
    };

    void SetIR(IR *ir) {
        m_ir = ir;
    }

    PadRing *GetPadRing() {
        return m_ring;
    };

    void SetTimer();
    Handle *GetTimer() {
        return &m_timer;
    };

    void Sampling(u32 rcpr, Remapper *remapper);
    void ReadFromIO(PadEntry *entry, uint32_t *raw, CirclePadEntry *circlepad, Remapper *remapper);
    Handle *GetEvent() {
        return &m_event;
    };

    uint32_t *GetLatestRawKeys() {
        return &m_rawkeys;
    }

private:
    uint8_t m_isinitialized = 0;
    uint32_t m_latestkeys = 0;
    uint32_t m_rawkeys = 0;
    uint32_t m_dummy = 0; // this is used for inputredirections
    Handle m_timer;
    PadRing *m_ring = nullptr;
    IR *m_ir = nullptr;
    Handle m_event;
    CirclePad m_circlepad;
    Slider m_slider;
    uint8_t m_counter;
};