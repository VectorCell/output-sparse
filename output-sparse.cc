/*
**  output-sparse.cc
**
**  Author: Brandon Smith
*/

#include "output-sparse.h"

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>

#include <cstdio>
#include <cstdint>

#include <unistd.h>
#include <inttypes.h>
#include <signal.h>

using namespace std;

const int BLOCKSIZE = 4096;

atomic<bool> stopped(false);
bool dry_run = false;
bool print_stats = false;
bool verbose = false;
string infilename = "-";
string outfilename = "-";

const vector<string> prefixes = {"", "ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi"};

void sighandler (int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		stopped = true;
	}
}

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

bool output_sparse(
	FILE *infile, FILE *outfile, 
	const size_t blocksize, size_t& bytes_written, size_t& n_skipped_blocks)
{
	if (infile == NULL)
		return false;

	vector<uint8_t> buffer(blocksize);
	void *buf = &buffer[0];
	n_skipped_blocks = 0;
	bytes_written = 0;
	size_t count = 0;
	while (!stopped && (count = fread(buf, 1, buffer.size(), infile)) > 0) {
		bytes_written += count;
		if (is_blk_zeroed(buf, count)) {
			++n_skipped_blocks;
			if (outfile != NULL)
				fseek(outfile, count, SEEK_CUR);
			if (verbose) {
				printf("hole at file index %zu\n", bytes_written);
			}
		} else {
			if (outfile != NULL)
				fwrite(buf, count, 1, outfile);
		}
	}
	if (outfile != NULL && 0 != ftruncate(fileno(outfile), bytes_written)) {
		fprintf(stderr, "ERROR: unable to truncate output file %s\n", outfilename.c_str());
		return false;
	}

	fclose(infile);
	if (outfile != NULL)
		fclose(outfile);

	return true;
}

bool parse_args (int argc, char *argv[], string& infilename, string& outfilename)
{
	int c;
	while ((c = getopt(argc, argv, "i:npv")) != -1) {
		switch (c) {
			case 'i':
				infilename = optarg;
				break;
			case 'n':
				dry_run = true;
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

	return true;
}

int main (int argc, char *argv[])
{
	if (!parse_args(argc, argv, infilename, outfilename))
		return EXIT_FAILURE;

	if (signal(SIGINT, sighandler) == SIG_ERR) {
		fprintf(stderr, "ERROR: unable to set handler for SIGINT.\n");
		return EXIT_FAILURE;
	}
	if (signal(SIGTERM, sighandler) == SIG_ERR) {
		fprintf(stderr, "ERROR: unable to set handler for SIGTERM.\n");
		return EXIT_FAILURE;
	}

	// open in/out files according to args
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
	if (!dry_run && outfilename == "-") {
		fprintf(stderr, "ERROR: unable to output to stdout (no sparse support)\n");
		return EXIT_FAILURE;
	} else {
		if (dry_run) {
			outfile = NULL;
		} else {
			outfile = fopen(outfilename.c_str(), "wb");
			if (outfile == NULL) {
				fprintf(stderr, "ERROR: unable to open file %s\n", outfilename.c_str());
				return EXIT_FAILURE;
			}
		}
	}

	if (verbose) {
		cout << "creating sparse file " << outfilename << endl;
	}

	// gets start time
	auto start = std::chrono::steady_clock::now();

	// writes sparse file
	size_t bytes_written, n_skipped_blocks;
	if (!output_sparse(infile, outfile, BLOCKSIZE, bytes_written, n_skipped_blocks))
		return EXIT_FAILURE;

	// gets end time
	auto stop = chrono::steady_clock::now();

	if (print_stats) {
		uint64_t elapsed_ms = chrono::duration_cast<chrono::milliseconds>(stop - start).count();
		if (elapsed_ms == 0)
			elapsed_ms = 1;
		uint64_t elapsed_min = elapsed_ms / 60000;
		double elapsed_s = (elapsed_ms - (elapsed_min * 60000)) / 1000.0;

		double base = bytes_written;
		int index = 0;
		while (base > 1024) {
			++index;
			base /= 1024;
		}
		printf("processed %.4g %sB in %llum%gs ", base, prefixes[index].c_str(), (long long unsigned)elapsed_min, elapsed_s);

		double rate = bytes_written / (elapsed_ms / 1000.0);
		index = 0;
		while (rate > 1024) {
			++index;
			rate /= 1024;
		}
		printf("(%.4g %sB/s)\n", rate, prefixes[index].c_str());

		float percent = 100 * static_cast<float>(n_skipped_blocks * BLOCKSIZE) / bytes_written;
		printf("skipped %zu data blocks, representing %g%% of total size\n", n_skipped_blocks, percent);
	}

	return EXIT_SUCCESS;
}
