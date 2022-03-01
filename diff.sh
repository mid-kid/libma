#!/bin/sh
# Build and compare the selected file, against a dump made with dump.sh
set -e
rm -f "$1.o"
make "$1.o"
objdump -xD "$1.o" > "$1.new"
diff -u "$1.dmp" "$1.new" | tee "$1.diff" | head -n 50
