#pragma once
#include <3ds.h>
#include <cstring>
#include "../CirclePad.hpp"
#include "exclusive_rw.hpp"

struct CPPEntry {
    int32_t currpadstate;
    int32_t pressedpadstate;
    int32_t releasedpadstate;
    CirclePadEntry circlepadstate;
};

static_assert(sizeof(CPPEntry) == 0x10, "Sizeof CPPEntry is not 0x10 bytes!");

class CPPRing {
public:
    CPPRing() {
        Reset();
    }

    void Reset() {
        m_tickcount = -1;
        m_oldtickcount = -1;
        m_updatedindex = -1;

        //memset(&m_entries, 0, sizeof(m_entries) * sizeof(CPPEntry));
    }

    CPPEntry GetLatest(uint8_t index) {
        return m_entries[index];
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