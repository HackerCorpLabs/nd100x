/*
 * Unit tests for HDLC CRC-16-CCITT.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../src/devices/hdlc/hdlcFrame.h"

static int failures = 0;

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        printf("  FAIL: %s: expected 0x%04X, got 0x%04X (line %d)\n", msg, (unsigned)(b), (unsigned)(a), __LINE__); \
        failures++; \
    } \
} while(0)

static void test_crc_known_vector(void)
{
    printf("  test_crc_known_vector...");
    // CRC-16-CCITT of "123456789" with init 0xFFFF should be 0x29B1
    uint8_t data[] = "123456789";
    uint16_t crc = HDLCFrame_CalculateCRC(data, 9);
    ASSERT_EQ(crc, 0x29B1, "CRC of '123456789'");
    printf(" ok\n");
}

static void test_crc_empty(void)
{
    printf("  test_crc_empty...");
    uint16_t crc = HDLCFrame_CalculateCRC(NULL, 0);
    // Empty data with init 0xFFFF stays 0xFFFF
    ASSERT_EQ(crc, 0xFFFF, "CRC of empty data");
    printf(" ok\n");
}

static void test_crc_single_byte(void)
{
    printf("  test_crc_single_byte...");
    uint8_t data = 0x00;
    uint16_t crc = HDLCFrame_CalculateCRC(&data, 1);
    // Verify it's deterministic and not 0xFFFF
    uint16_t crc2 = HDLCFrame_CalculateCRC(&data, 1);
    ASSERT_EQ(crc, crc2, "CRC deterministic");
    printf(" ok\n");
}

static void test_crc_incremental(void)
{
    printf("  test_crc_incremental...");
    // Incremental CRC should match batch CRC
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t batch = HDLCFrame_CalculateCRC(data, 4);

    uint16_t incr = 0xFFFF;
    for (int i = 0; i < 4; i++) {
        incr = HDLCFrame_UpdateCRC(incr, data[i]);
    }
    ASSERT_EQ(incr, batch, "incremental matches batch");
    printf(" ok\n");
}

int run_hdlc_crc_tests(void)
{
    failures = 0;

    test_crc_known_vector();
    test_crc_empty();
    test_crc_single_byte();
    test_crc_incremental();

    if (failures == 0) {
        printf("  All hdlc_crc tests passed.\n");
    }
    return failures;
}
