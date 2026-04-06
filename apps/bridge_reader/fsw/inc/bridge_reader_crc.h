/************************************************************************
 * CRC-16/CCITT-FALSE — matches rust-bridge compute_crc16_ccitt.
 ************************************************************************/
#ifndef BRIDGE_READER_CRC_H
#define BRIDGE_READER_CRC_H

#include <stddef.h>
#include <stdint.h>

uint16_t BridgeReader_Crc16CcittFalse(const uint8_t *data, size_t len);

#endif
