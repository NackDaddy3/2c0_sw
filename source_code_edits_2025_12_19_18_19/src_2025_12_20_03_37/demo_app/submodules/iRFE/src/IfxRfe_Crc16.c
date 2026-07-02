/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#include "IfxRfe_Crc16.h"

uint16_t crc16_ccitt_false(const uint8_t buf[], uint16_t len, uint16_t crc)
{
    while (len--)
    {
        uint8_t x = (crc >> 8) ^ *buf++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t)x << 12) ^ ((uint16_t)x << 5) ^ x;
    }
    return crc;
}
