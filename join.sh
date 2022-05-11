#!/bin/sh
# Link all objects into a single relocatable object file, useful for analysis in ghidra.
thumb-elf-ld -r -d -o libma.o *.o
