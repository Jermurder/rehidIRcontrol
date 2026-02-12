#pragma once
#include <3ds.h>

uint8_t crc8(const void *buf, size_t len);
uint8_t crc8_loop(const uint8_t *container_start, size_t container_size, const uint8_t *segment, size_t segment_size);