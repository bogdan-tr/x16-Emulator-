#!/bin/bash
#
# Compile it
./xas giza.x16s
# Run the test with the provided argument
printf "$1\n" | ./x16 a.obj
