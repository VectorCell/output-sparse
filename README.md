# output-sparse

Tool for creating a sparse copy of an existing file.

Usage:
```
output-sparse [-p] [-v] [-i INPUTFILE] OUTPUTFILE
```

If no input file is specified, input is read from stdin.

```
FLAGS

	-n
		performs a dry run (doesn't write anything to disk)

	-p
		print stats to stdout upon completion

	-v
		verbose mode, implies -p
```
