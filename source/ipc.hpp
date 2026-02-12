#pragma once
#include <3ds.h>
#include "hid.hpp"

namespace IPC {
    void HandleHidCommands(Hid *hid);
    void HandleMockIRUserCommands(Hid *hid);
};