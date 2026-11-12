# Build the stitcher with nvcc (Google Colab GPU runtime).
# Authors: NDB
#
# make                          build ./stitch
# make clean                    remove it
# make submission GROUP=<n>     zip the notebook + src/ + tools/ as Group<n>_Midterm.zip
# make test                     unit tests of stages 4-6 with g++ (no GPU needed)
#
# -O2 applies to host code too. No -march=native / -fopenmp, so V0 stays a fair
# single-thread baseline.
#
# The notebook does not use this file: its setup cell runs the same command itself
# (BUILD = 'nvcc -O2 -std=c++17 -o stitch src/*.cu src/*.cpp'). Keep the two in sync.

NVCC      ?= nvcc
NVCCFLAGS ?= -O2 -std=c++17

SRCS := $(wildcard src/*.cu) $(wildcard src/*.cpp)
HDRS := $(wildcard src/*.h) $(wildcard src/*.cuh)

# The notebook keeps its name until Day 20 (GroupXX_...), then gets the group number.
NOTEBOOK ?= $(firstword $(wildcard Group*_Midterm.ipynb))

stitch: $(SRCS) $(HDRS)
	$(NVCC) $(NVCCFLAGS) -o $@ $(SRCS)

clean:
	rm -f stitch
	rm -rf build

# Unit tests (tests/*.cpp): plain C++, built with the host compiler, so they also run
# on a laptop without CUDA.
CXX       ?= g++
TESTFLAGS ?= -O2 -std=c++17 -Isrc

test:
	mkdir -p build
	$(CXX) $(TESTFLAGS) -o build/test_match tests/test_match.cpp src/v0_match.cpp
	$(CXX) $(TESTFLAGS) -o build/test_homography tests/test_homography.cpp \
		src/v0_homography.cpp src/ransac_samples.cpp src/v0_warp.cpp
	./build/test_match
	./build/test_homography

# Midterm package (stage1_midterm_plan.md section 1): the notebook, saved with all
# outputs, renamed to Group<n>_Midterm.ipynb, plus src/. tools/ goes in too: the
# result cells call tools/*.py. The notebook builds with nvcc itself, so no Makefile.
submission:
	@test -n "$(GROUP)" || { echo "Usage: make submission GROUP=<n>"; exit 1; }
	@test -n "$(NOTEBOOK)" || { echo "No Group*_Midterm.ipynb found"; exit 1; }
	rm -rf .submission Group$(GROUP)_Midterm.zip
	mkdir .submission
	cp "$(NOTEBOOK)" .submission/Group$(GROUP)_Midterm.ipynb
	cp -r src tools .submission/
	rm -rf .submission/tools/__pycache__
	cd .submission && zip -qr ../Group$(GROUP)_Midterm.zip Group$(GROUP)_Midterm.ipynb src tools
	rm -rf .submission
	@echo "Wrote Group$(GROUP)_Midterm.zip"

.PHONY: clean submission test
