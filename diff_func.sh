#!/bin/sh
# Build the selected file, compare a single function
set -e
rm -f "$1.o"
make "$1.o"
LANG=C thumb-elf-objdump -xD "$1.o" > "$1.new"

filter_func() {
    awk -v RS= "/^[0-9a-f]+ <$1>:/" | \
        sed -e 's/^[ 	]*[0-9a-f]\+://' \
            -e '/^ *R_ARM_/d' \
            -e "s/[0-9a-f]\+ <\(.*\)>/\1/"
}
cat "$1.dmp" | filter_func "$2" > "$1.$2.dmp"
cat "$1.new" | filter_func "$2" > "$1.$2.new"
diff -u "$1.$2.dmp" "$1.$2.new"
