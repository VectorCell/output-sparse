CSTD   := c11
CPPSTD := c++11

ifeq "$(CXX)" "g++"
	GCCVERSIONLT48 := $(shell expr `gcc -dumpversion` \< 4.8)
	ifeq "$(GCCVERSIONLT48)" "1"
		CC  := gcc-4.8
		CXX := g++-4.8
	endif
endif

CFLAGS   := -pedantic -std=$(CSTD) -Wall -Werror -O3
CPPFLAGS := -pedantic -std=$(CPPSTD) -Wall -Werror -O3
LIBFLAGS := -fopenmp

all: output-sparse

output-sparse : output-sparse.cc
	$(CXX) $(CPPFLAGS) -o output-sparse output-sparse.cc $(LIBFLAGS)

test-output-sparse : output-sparse
	./test-output-sparse.sh

test : test-output-sparse

install :
	@./install.sh

clean :
	rm -f *.o
	rm -f *.d
	rm -f *.elf
	rm -f output-sparse

-include *.d
