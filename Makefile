# Build the stitcher with nvcc (Google Colab GPU runtime).
# Authors: NDB
#
# make          build ./stitch
# make clean    remove it
#
# -O2 applies to host code too. No -march=native / -fopenmp, so V0 stays a fair
# single-thread baseline.

NVCC      ?= nvcc
NVCCFLAGS ?= -O2 -std=c++17

SRCS := $(wildcard src/*.cu) $(wildcard src/*.cpp)
HDRS := $(wildcard src/*.h) $(wildcard src/*.cuh)

stitch: $(SRCS) $(HDRS)
	$(NVCC) $(NVCCFLAGS) -o $@ $(SRCS)

clean:
	rm -f stitch

.PHONY: clean
