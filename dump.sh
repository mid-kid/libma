#!/bin/sh
# Dump all objects into a readable text format
for x in *.o; do
    LANG=C thumb-elf-objdump -xD "$x" > "$(basename "$x" .o).dmp"
done
