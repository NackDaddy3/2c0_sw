/*
 * (c) (2022-2025), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */

#include "sequencer_data.h"

/**
 * @brief Primary sequencer data array
 * 
 * This array contains the radar timing and control sequences for the primary CTRX device.
 * The ramp scenario consists of 512 ramps
 */
const uint32_t seqData_prim[SEQUENCER_PROGRAM_LEN] = {0x31000020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01000000, 0xc00000ac, 0x00000004, 0xc4020000, 0x47800000, 0x00000fa0, 0x0380a3d0, 0x00000000, 0x00010000, 0x10000000, 0xe0010200, 0x43800000, 0x00000190, 0x0380a3e0, 0x001b5000, 0x00030400, 0x42000000, 0x00001000, 0x0000c500, 0x42000000, 0x00000008, 0x00000500, 0x43000000, 0x000000c8, 0x3d999800, 0x00b00000, 0x43000000, 0x0000025c, 0x00000000, 0x00b00000, 0x98000150, 0xe0020000, 0x47000000, 0x000000c8, 0x00000000, 0x80000000, 0x00028000, 0xffffffff, 0x10028001, 0x10028000, 0x10028004, 0x10028000};

/**
 * @brief Secondary sequencer data array
 * 
 * This array contains the radar timing and control sequences for the secondary CTRX device.
 * The ramp scenario consists of 512 ramps
 */
const uint32_t seqData_sec[SEQUENCER_PROGRAM_LEN] = {0x31000020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01000000, 0xc00000ac, 0x00000004, 0xc0020000, 0x47800000, 0x00000fa0, 0x0380a3d0, 0x00000000, 0x00010100, 0x10000000, 0xe0010200, 0x43800000, 0x00000190, 0x0380a3d0, 0x00000000, 0x00030400, 0x42000000, 0x00001000, 0x0000c500, 0x42000000, 0x00000008, 0x00000500, 0x43000000, 0x000000c8, 0x00000000, 0x00b00000, 0x43000000, 0x0000025c, 0x00000000, 0x00b00000, 0x98000150, 0xe0020000, 0x47000000, 0x00000064, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x10000000, 0x10000002, 0x10000000, 0x10000008};