#include <iostream>
#include <vector>
#include <string>

#include <cstdio>
#include <cstdint>

#include <unistd.h>

using namespace std;

const int BLOCKSIZE = 4096;

bool print_stats = false;
bool verbose = false;
string infilename = "-";
string outfilename = "-";

bool is_blk_zeroed (const void *block, size_t size)
{
	const uint8_t *ptr = static_cast<const uint8_t*>(block);
	for (size_t k = 0; k < size; ++k) {
		if (ptr[k] != 0) {
			return false;
		}
	}
	return true;
}

bool output_sparse(FILE *infile, FILE *outfile, size_t& bytes_written, size_t& n_skipped_blocks)
{
	if (infile == NULL || outfile == NULL)
		return false;

	vector<uint8_t> buffer(BLOCKSIZE);
	void *buf = &buffer[0];
	n_skipped_blocks = 0;
	bytes_written = 0;
	size_t count = 0;
	while ((count = fread(buf, 1, BLOCKSIZE, infile)) > 0) {
		bytes_written += count;
		if (is_blk_zeroed(buf, count)) {
			++n_skipped_blocks;
			fseek(outfile, count, SEEK_CUR);
			if (verbose) {
				printf("hole at file index %lu\n", bytes_written);
			}
		} else {
			fwrite(buf, count, 1, outfile);
		}
	}
	if (0 != ftruncate(fileno(outfile), bytes_written)) {
		fprintf(stderr, "ERROR: unable to truncate output file %s\n", outfilename.c_str());
		return false;
	}
	fclose(outfile);
	return true;
}

bool parse_args (int argc, char *argv[], string& infilename, string& outfilename)
{
	int c;
	while ((c = getopt(argc, argv, "i:pv")) != -1) {
		switch (c) {
			case 'i':
				infilename = optarg;
				break;
			case 'p':
				print_stats = true;
				break;
			case 'v':
				verbose = true;
				break;
			default:
				fprintf(stderr, "unknown arg: %c\n", c);
				return false;
		}
	}

	for (int k = optind; k < argc; ++k) {
		outfilename = argv[k];
	}

	if (verbose)
		print_stats = true;

	if (verbose) {
		cout << "infilename:  " << infilename << endl;
		cout << "outfilename: " << outfilename << endl;
		cout << "print_stats: " << (print_stats ? "true" : "false") << endl;
		cout << "verbose:     " << (verbose ? "true" : "false") << endl;
	}
	return true;
}

int main (int argc, char *argv[])
{
	if (!parse_args(argc, argv, infilename, outfilename))
		return EXIT_FAILURE;

	FILE *infile;
	FILE *outfile;
	if (infilename == "-") {
		infile = stdin;
		infilename = "stdin";
	} else {
		infile = fopen(infilename.c_str(), "rb");
		if (infile == NULL) {
			fprintf(stderr, "ERROR: unable to open file %s\n", infilename.c_str());
			return EXIT_FAILURE;
		}
	}
	if (outfilename == "-") {
		fprintf(stderr, "ERROR: unable to output to stdout (no sparse support)\n");
		return EXIT_FAILURE;
	} else {
		outfile = fopen(outfilename.c_str(), "wb");
		if (outfile == NULL) {
			fprintf(stderr, "ERROR: unable to open file %s\n", outfilename.c_str());
			return EXIT_FAILURE;
		}
	}

	if (verbose) {
		cout << "creating sparse file " << outfilename << endl;
	}

	size_t bytes_written, n_skipped_blocks;
	if (!output_sparse(infile, outfile, bytes_written, n_skipped_blocks))
		return EXIT_FAILURE;

	if (print_stats) {
		cout << "processed " << bytes_written << " bytes" << endl;
		cout << "skipped " << n_skipped_blocks << " data blocks" << endl;
		float percent = 100 * static_cast<float>(n_skipped_blocks * BLOCKSIZE) / bytes_written;
		cout << "saved " << percent << "% of data blocks" << endl;
	}

	return EXIT_SUCCESS;
}
