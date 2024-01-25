/**
 * @file commands.h
 * @author Max Bensink (maxbensink@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-10-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ADS1258_H_
#define ADS1258_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

//*****************************************************************************
//
// Constants
//
//*****************************************************************************
constexpr uint8_t NUM_REGISTERS = 10;

enum RegisterAdressses
{
    CONFIG0 = 0x0,
    CONFIG1,
    MUXSCH,
    MUXDIF,
    MUXSG0,
    MUXSG1,
    SYSRED,
    GPIOC,
    GPIOD,
    ID
};

union Command
{
    struct
    {
        char address : 4;
        bool multiple : 1;
        char command : 3;
    } bits;

    char data;
};

/* Command byte definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |            C[2:0]           |   MUL   |                A[3:0]                 |
 * ---------------------------------------------------------------------------------
 */

union CommandByte
{
    struct
    {
        char address : 4;
        bool multiple : 1;
        char command : 3;
    } bits;

    char raw_data;
};

/* SPI Commands */
enum Commands : char
{
    READ_DIRECT = 0x0,
    READ_COMMAND = 0x1,
    READ_REGISTERS = 0x2,
    WRITE_REGISTERS = 0x3,
    PULSE_CONVERT = 0x4,
    RESET = 0x6
};

//*****************************************************************************
//
// Status byte formatting
//
//*****************************************************************************

/* STATUS byte definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |   NEW   |   OVF   |  SUPPLY |                    CHID[4:0]                    |
 * ---------------------------------------------------------------------------------
 */

union StatusByte
{
    struct
    {
        char CHID : 5;   // Bits 0-4
        bool SUPPLY : 1; // Bit 5
        bool OVF : 1;    // Bit 6
        bool NEW : 1;    // Bit 7
    } bits;

    char raw_data; // The full byte for direct access
};

enum ChannelIdentifiers
{
    DIFF0 = 0x00,
    DIFF1,
    DIFF2,
    DIFF3,
    DIFF4,
    DIFF5,
    DIFF6,
    DIFF7,
    AIN0,
    AIN1,
    AIN2,
    AIN3,
    AIN4,
    AIN5,
    AIN6,
    AIN7,
    AIN8,
    AIN9,
    AIN10,
    AIN11,
    AIN12,
    AIN13,
    AIN14,
    AIN15,
    OFFSET,
    VCC,
    TEMP,
    GAIN,
    REF,
    FIXEDCHMODE
};

//*****************************************************************************
//
// Register definitions
//
//*****************************************************************************

/* Register 0x00 (CONFIG0) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |    0    |  SPIRST |  MUXMOD |  BYPAS  |  CLKENB |   CHOP  |   STAT  |    0    |
 * ---------------------------------------------------------------------------------
 */

union Config0
{
    struct
    {
        const bool zero1 : 1 = 0; // Bit 0, fixed at '0'
        bool stat : 1;            // Bit 1
        bool chop : 1;            // Bit 2
        bool clken : 1;           // Bit 3
        bool bypass : 1;          // Bit 4
        bool muxmod : 1;          // Bit 5
        bool spirst : 1;          // Bit 6
        const bool zero2 : 1 = 0; // Bit 7, fixed at '0'
    } bits;

    char raw_data; // The full byte for direct access
};

constexpr Config0 CONFIG0_DEFAULT = {.raw_data = 0x0A};

/* Register 0x01 (CONFIG1; definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |  IDLMOD |           DLY[2:0]          |     SCBCS[1:0]    |     DRATE[0:1]    |
 * ---------------------------------------------------------------------------------
 */

union Config1
{
    struct
    {
        char data_rate : 2; // Bits 0-1
        char scbcs : 2;     // Bits 2-3
        char delay : 3;     // Bits 4-6
        bool idle_mode : 1; // Bit 7
    } bits;

    char raw_data; // The full byte for direct access
};

/** CONFIG1 default (reset) value */
constexpr Config1 CONFIG1_DEFAULT = {.raw_data = 0x83};

/* DLY field values */
enum DelayConfig
{
    DLY_0us = 0x00,
    DLY_8us,
    DLY_16us,
    DLY_32us,
    DLY_64us,
    DLY_128us,
    DLY_256us,
    DLY_384us
};

/* SCBCS field values */
enum ScbcsConfig
{
    SCBCS_OFF = 0x00,
    SCBCS_1_5uA,
    SCBCS_24uA
};

/* DRATE field values (fixed-channel DRs shown) */
enum DrateConfig
{
    DRATE_0 = 0x00,
    DRATE_1,
    DRATE_2,
    DRATE_3
};

/* Register 0x02 (MUXSCH) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |               AINP[3:0]               |               AINN[3:0]               |
 * ---------------------------------------------------------------------------------
 */

union Muxsch
{
    struct
    {
        char AINN : 4; // Bits 0-3
        char AINP : 4; // Bits 4-7
    } bits;

    char raw_data; // The full byte for direct access
};

/* MUXSCH default */
constexpr Muxsch MUXSCH_DEFAULT = {.raw_data = 0x00};

/* Register 0x03 (MUXDIF) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |  DIFF7  |  DIFF6  |  DIFF5  |  DIFF4  |  DIFF3  |  DIFF2  |  DIFF1  |  DIFF0  |
 * ---------------------------------------------------------------------------------
 */

union Muxdif
{
    struct
    {
        bool DIFF0 : 1; // Bit 0
        bool DIFF1 : 1; // Bit 1
        bool DIFF2 : 1; // Bit 2
        bool DIFF3 : 1; // Bit 3
        bool DIFF4 : 1; // Bit 4
        bool DIFF5 : 1; // Bit 5
        bool DIFF6 : 1; // Bit 6
        bool DIFF7 : 1; // Bit 7
    } bits;

    char raw_data; // The full byte for direct access
};

/** MUXDIF default (reset; value */
constexpr Muxdif MUXDIF_DEFAULT = {.raw_data = 0x00};

/* Register 0x04 (MUXSG0) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |   AIN7  |   AIN6  |   AIN5  |   AIN4  |   AIN3  |   AIN2  |   AIN1  |   AIN0  |
 * ---------------------------------------------------------------------------------
 */

union Muxsg0
{
    struct
    {
        bool AIN0 : 1; // Bit 0
        bool AIN1 : 1; // Bit 1
        bool AIN2 : 1; // Bit 2
        bool AIN3 : 1; // Bit 3
        bool AIN4 : 1; // Bit 4
        bool AIN5 : 1; // Bit 5
        bool AIN6 : 1; // Bit 6
        bool AIN7 : 1; // Bit 7
    } bits;

    char raw_data; // The full byte for direct access
};

/** MUXSG0 default (reset) value */
constexpr Muxsg0 MUXSG0_DEFAULT = {.raw_data = 0xFF};

/* Register 0x05 (MUXSG1) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |  AIN15  |  AIN14  |  AIN13  |  AIN12  |  AIN11  |  AIN10  |   AIN9  |   AIN8  |
 * ---------------------------------------------------------------------------------
 */

union Muxsg1
{
    struct
    {
        bool AIN8 : 1;  // Bit 0
        bool AIN9 : 1;  // Bit 1
        bool AIN10 : 1; // Bit 2
        bool AIN11 : 1; // Bit 3
        bool AIN12 : 1; // Bit 4
        bool AIN13 : 1; // Bit 5
        bool AIN14 : 1; // Bit 6
        bool AIN15 : 1; // Bit 7
    } bits;

    char raw_data; // The full byte for direct access
};

/** MUXSG1 default (reset) value */
constexpr Muxsg1 MUXSG1_DEFAULT = {.raw_data = 0xFF};

/* Register 0x06 (SYSRED) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |    0    |    0    |   REF   |   GAIN  |   TEMP  |   VCC   |     0   |  OFFSET |
 * ---------------------------------------------------------------------------------
 */

union Sysred
{
    struct
    {
        bool offset : 1;          // Bit 0
        const bool zero1 : 1 = 0; // Bit 1, fixed at '0'
        bool vcc : 1;             // Bit 2
        bool temp : 1;            // Bit 3
        bool gain : 1;            // Bit 4
        bool ref : 1;             // Bit 5
        const bool zero2 : 2 = 0; // Bits 6-7, fixed at '0'
    } bits;

    char raw_data; // The full byte for direct access
};

/** SYSRED default (reset) value */
constexpr Sysred SYSRED_DEFAULT = {.raw_data = 0x00};

/* Register 0x07 & 0x08 (GPIOC & GPIOD) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |                                    CIO[7:0]                                   |
 * ---------------------------------------------------------------------------------
 */

union GpioReg
{
    struct
    {
        bool gpio1 : 1;
        bool gpio2 : 1;
        bool gpio3 : 1;
        bool gpio4 : 1;
        bool gpio5 : 1;
        bool gpio6 : 1;
        bool gpio7 : 1;
        bool gpio8 : 1;
    } bits;

    char raw_data; // The full byte for direct access
};

typedef GpioReg Gpioc;
typedef GpioReg Gpiod;

/** GPIOC default (reset) value */
constexpr Gpioc GPIOC_DEFAULT = {.raw_data = 0xFF};

/** GPIOD default (reset) value */
constexpr Gpiod GPIOD_DEFAULT = {.raw_data = 0x00};

/* Register 0x09 (ID) definition
 * ---------------------------------------------------------------------------------
 * |  Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0  |
 * ---------------------------------------------------------------------------------
 * |                                    ID[7:0]                                    |
 * ---------------------------------------------------------------------------------
 */

// Define a union that represents your ID register
union IdReg
{

    struct
    {
        const char start : 4;
        const char type : 1;
        const char end : 3;
    } bits;

    char raw_data; // The full byte for direct access
};

/* ID4 field values */
constexpr uint8_t ADS1258_ID = 0x00;
constexpr uint8_t ADS1158_ID = 0x10;

#endif /* ADS1258_H_ */
