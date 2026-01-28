#pragma once
#include <3ds.h>
struct CirclePadEntry {
    int16_t x;
    int16_t y;
};

enum CirclePadMode {
    CPAD,
    CSTICK
};

class CirclePad {
public:
    void GetConfigSettings();
    void RawToCirclePadCoords(CirclePadEntry *result, CirclePadEntry raw);

    template<CirclePadMode N> static uint32_t ConvertToHidButtons(CirclePadEntry *circlepad, uint32_t buttons) {
        constexpr uint32_t cpadkeys[4] {KEY_CPAD_LEFT, KEY_CPAD_RIGHT, KEY_CPAD_UP, KEY_CPAD_DOWN};
        constexpr uint32_t cstickkeys[4] {KEY_CSTICK_LEFT, KEY_CSTICK_RIGHT, KEY_CSTICK_UP, KEY_CSTICK_DOWN};
        return ConvertToHidButtons(circlepad, buttons, N == CPAD ? &cpadkeys[0] : &cstickkeys[0]);
    }

    static uint32_t ConvertToHidButtons(CirclePadEntry *circlepad, uint32_t buttons, const uint32_t *keys);
    static void AdjustValues(int16_t *adjustedx, int16_t *adjustedy, int rawx, int rawy, int min, int max);
private:
    CirclePadEntry m_latestdata = {0x800, 0x800};
    CirclePadEntry m_center = {0x800, 0x800};
    float m_scalex = 1.0f;
    float m_scaley = 1.0f;
};