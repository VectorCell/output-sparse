# output-sparse

Tool for creating a sparse file from another file or data stream. Useful for 
keeping disk images small while still maintaining disk availability for 
virtualized clients.

This tool can only create sparse files if used with operating and file systems 
that have sparse file support.

Usage:
```
output-sparse [FLAGS] [-i INPUTFILE] OUTPUTFILE
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

This tool was designed primarily with disk images in mind. Decompressing an 
existing disk image may take up a large amount of disk space, even if most of 
the virtual sectors are unused and zeroed out. Decompressing a disk image to a 
sparse file can save a lot of disk space and prevent unnecessary disk I/O.
