#include <3ds.h>

u32 IR_GetButtons();
int IR_GetStickX();
int IR_GetStickY();
bool IR_IsActive();

extern bool g_receiveActive;
extern bool usingIR;

void initIR();
void startReceive();
void pollReceive();
void irExit();