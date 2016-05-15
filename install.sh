#!/bin/bash

if [ ! -f output-sparse ]; then
	echo "ERROR: must run make first"
	exit 1
fi

echo "copying output-sparse to ~/bin/output-sparse"
cp output-sparse ~/bin/
