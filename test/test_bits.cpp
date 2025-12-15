#include "catch.hpp"

extern "C" {
#include "bits.h"
}

// ------------------------ Test bits ------------------------------

TEST_CASE("Bits.getbit", "[bits]") {
    uint16_t val = 0x1;
    REQUIRE(getbit(val, 0) == 1);

    val = 0x20;
    REQUIRE(getbit(val, 5) == 1);
    REQUIRE(getbit(val, 4) == 0);
}

TEST_CASE("Bits.getbits", "[bits]") {
    uint16_t val = 0x1;
    REQUIRE(getbits(val, 0, 2) == 1);

    val = 0x30;
    REQUIRE(getbits(val, 4, 2) == 3);
    REQUIRE(getbits(val, 3, 2) == 2);
}

TEST_CASE("Bits.setbit", "[bits]") {
    uint16_t val = 0x1;
    val = setbit(val, 2);
    REQUIRE(val == 5);
}

TEST_CASE("Bits.clearbit", "[bits]") {
    uint16_t val = 0x5;
    val = clearbit(val, 2);
    REQUIRE(val == 1);
}


TEST_CASE("Bits.signextend", "[bits]") {
    uint16_t input  = 0x12;
    REQUIRE(sign_extend(input, 5) == 65522);
    REQUIRE(sign_extend(input, 6) == 0x12);
}

TEST_CASE("Bits.is_posneg", "[bits]") {
    REQUIRE(is_positive(0x5) == true);
    REQUIRE(is_positive(65522) == false);
    REQUIRE(is_negative(0x032f) == false);
    REQUIRE(is_negative(0xe32f) == true);
}
