#pragma once
#include <3ds.h>
#include "exclusive_rw.hpp"

struct CPPEntry {
    int32_t currpadstate;
    int32_t pressedpadstate;
    int32_t releasedpadstate;
    int16_t pad0; // circlepad x
    int16_t pad1; // circlepad y
};

static_assert(sizeof(CPPEntry) == 0x10, "Sizeof CPPEntry is not 0x10 bytes!");

class CPPRing {
public:
    CPPRing() {
        Reset();
    }

    void Reset() {
        m_tickcount = 0;
        m_oldtickcount = 0;
        m_updatedindex = 0;
    }

    int32_t GetLatest(uint8_t index) {
        return m_entries[index].currpadstate;
    };

    int32_t GetIndex() {
        return m_updatedindex;
    };

    int64_t GetTickCount() {
        return m_tickcount;
    };

    int64_t GetOldTickCount() {
        return m_oldtickcount;
    };

    void WriteToRing(CPPEntry *entry);
private:
    int64_t m_tickcount = 0;
    int64_t m_oldtickcount = 0;
    int32_t m_updatedindex = 0;
    CPPEntry m_entries[8];
};

static_assert(sizeof(CPPRing) == 0x98, "Size of CPPRing is not 0x98 bytes!");