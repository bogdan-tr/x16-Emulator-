#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#define ASSERT_EQ(a, b)     REQUIRE((a) == (b))
#define ASSERT_TRUE(a)      REQUIRE(a)
#define ASSERT_FALSE(a)     REQUIRE(!(a))

extern "C" {
#include "bits.h"
#include "x16.h"
#include "control.h"
#include "instruction.h"
}

TEST_CASE("Memory.x16", "[memory]") {
    // --errors-for-leak-kinds=definite,indirect,possible,reachable
    // ./wwcached"
    printf("Testing leaks for X16 emulator...\n");
#ifdef __APPLE__
    const char* cmd = 
        "leaks --atExit -- ./x16 test/samples/test.obj";
#else
    const char* cmd = 
        "valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./x16 test/samples/test.obj";
#endif
    int status = system(cmd);
    REQUIRE(status == 0);
    printf("Success\n");
}


TEST_CASE("Memory.xas", "[memory]") {
    printf("Testing leaks for XAS assembler...\n");
#ifdef __APPLE__
    const char* cmd2 = 
        "leaks --atExit -- ./xas test/samples/test.s";
#else
    const char* cmd2 = 
        "valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./xas test/samples/test.s";
#endif
    int status2 = system(cmd2);
    REQUIRE(status2 == 0);
    printf("Success\n");    
}
