#!/bin/bash

echo
echo "expecting error"
./output-sparse
if [ "$?" == "0" ]; then
	echo "program exited with error code 0, which was unexpected"
	exit 1
fi

echo
echo "expecting error"
./output-sparse -vi input.img output.img
if [ "$?" == "0" ]; then
	echo "program exited with error code 0, which was unexpected"
	exit 1
fi

echo
dd if=/dev/zero of=input.img bs=4k count=4 2> /dev/null
./output-sparse -vi input.img output.img
rm input.img output.img

echo
dd if=/dev/zero of=input.img bs=4k count=4k 2> /dev/null
./output-sparse -pi input.img output.img
rm input.img output.img

