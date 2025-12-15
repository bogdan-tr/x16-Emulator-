#include "catch.hpp"
#include <iostream>
using namespace std;

// Test a simple one line case
TEST_CASE("Xas.simple", "[xas]") {
    cout << "Testing simple one line assembler...";

    int rv = system("./xas test/samples/simple.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/simple.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with comments
TEST_CASE("Xas.comment", "[xas]") {
    cout << "Testing comments in assembler...";

    int rv = system("./xas test/samples/comment.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/comment.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with with unused prior label instructions
TEST_CASE("Xas.prior", "[xas]") {
    cout << "Testing unused label in assembler...";

    int rv = system("./xas test/samples/prior.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/prior.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with multiple instructions
TEST_CASE("Xas.multi", "[xas]") {
    cout << "Testing multiple instructions in assembler...";

    int rv = system("./xas test/samples/multi.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/multi.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with forward labels
TEST_CASE("Xas.forward", "[xas]") {
    cout << "Testing forward labels in assembler...";

    int rv = system("./xas test/samples/forward.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/forward.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with loop program
TEST_CASE("Xas.loop", "[xas]") {
    cout << "Testing a full program with loops in assembler... ";

    int rv = system("./xas test/samples/loop.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/loop.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);
    cout << " assembler ok... ";

    // Run the program and test the output
    cout << "actually running the program...";
    rv = system("./x16 a.obj > out");
    REQUIRE(WEXITSTATUS(rv) == 0);
    rv = system("cmp out test/samples/loop-out");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with all instructions program
TEST_CASE("Xas.all", "[xas]") {
    cout << "Testing all instructions in assembler... ";

    int rv = system("./xas test/samples/all.x16s");
    REQUIRE(WEXITSTATUS(rv) == 0);

    rv = system("cmp a.obj test/samples/all.obj");
    REQUIRE(WEXITSTATUS(rv) == 0);

    cout << "Passed" << endl;
}

// Test with errors in assembler - missing label
TEST_CASE("Xas.error.nolabel", "[xas]") {
    cout << "Testing error with no matching label in assembler... ";

    int rv = system("./xas test/samples/error-nolabel.x16s");
    REQUIRE(WEXITSTATUS(rv) == 2);

    cout << "Passed" << endl;
}

// Test with errors in assembler - missing % before a reg
TEST_CASE("Xas.error.reg", "[xas]") {
    cout << "Testing error with no % before reg in assembler... ";

    int rv = system("./xas test/samples/error-reg.x16s");
    REQUIRE(WEXITSTATUS(rv) == 2);

    cout << "Passed" << endl;
}

// Test with errors in assembler - missing $ before a value
TEST_CASE("Xas.error.imm", "[xas]") {
    cout << "Testing error with no $ before value in assembler... ";

    int rv = system("./xas test/samples/error-val.x16s");
    REQUIRE(WEXITSTATUS(rv) == 2);

    cout << "Passed" << endl;
}
