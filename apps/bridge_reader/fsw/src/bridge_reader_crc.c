/************************************************************************
 * CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) — same algorithm as Rust bridge.
 ************************************************************************/

#include "bridge_reader_crc.h"

uint16_t BridgeReader_Crc16CcittFalse(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFu;
    size_t   i;
    int      b;

    for (i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (b = 0; b < 8; b++)
        {
            if ((crc & 0x8000u) != 0u)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}
