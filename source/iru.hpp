#pragma once

/* These are patched commands which returns stuff that we require from ir */

Result iruInit_();
void iruExit_(void);
Result IRU_Initialize_(void);
uint32_t IRU_GetLatestKeysPA_();
uint32_t IRU_GetStatePA_();
Result IRU_Shutdown_(void);