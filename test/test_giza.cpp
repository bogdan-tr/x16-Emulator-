#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
using namespace std;

#include "catch.hpp"

// Run the command on the system, and return the output
// This redirects the output of the command to the file named "out",
// then reads the entire file in and returns the data as a string
string runcheck(string cmd) {
    cout << "Running cmd: " + cmd << endl;

    // Send output to file
    cmd += " > out";
    system(cmd.c_str());

    // Read the file out and return its contents
    ifstream is("out");
    string data;
    getline(is, data, '\0');
    return data;
}

// Test a simple one line case
TEST_CASE("Giza", "[giza]") {
    cout << "Testing XAS with giza.x16s..." << endl;

    REQUIRE(runcheck("test/test_giza.sh 1") == "1\n[*]\nHALT\n\n");
    REQUIRE(runcheck("test/test_giza.sh 2") == "2\n[**]\nHALT\n\n");
    REQUIRE(runcheck("test/test_giza.sh 9") == "9\n[*********]\nHALT\n\n");
    REQUIRE(runcheck("test/test_giza.sh 0") == "0\n[]\nHALT\n\n");
    REQUIRE(runcheck("test/test_giza.sh 12") == "12\n[************]\nHALT\n\n");
    REQUIRE(runcheck("test/test_giza.sh 23") == "23\n[***********************]\nHALT\n\n");

    cout << "Success" << endl;
}
