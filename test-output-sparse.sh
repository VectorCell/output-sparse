#!/bin/bash

echo
echo "expecting error"
./output-sparse

echo
echo "expecting error"
./output-sparse -vi input.img output.img

echo
dd if=/dev/zero of=input.img bs=4k count=4 2> /dev/null
./output-sparse -vi input.img output.img
rm input.img output.img

