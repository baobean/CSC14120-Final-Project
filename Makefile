# Build the stitcher with nvcc (Google Colab GPU runtime).
# Authors: NDB
#
# make                          build ./stitch
# make clean                    remove it
# make submission GROUP=<n>     zip the notebook + src/ (+ Makefile, tools/) as Group<n>_Midterm.zip
#
# -O2 applies to host code too. No -march=native / -fopenmp, so V0 stays a fair
# single-thread baseline.

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

# Midterm package (stage1_midterm_plan.md section 1): the notebook, saved with all
# outputs, renamed to Group<n>_Midterm.ipynb, plus src/. Makefile and tools/ go in too:
# the notebook's setup cell builds with `make` and the result cells call tools/*.py.
submission:
	@test -n "$(GROUP)" || { echo "Usage: make submission GROUP=<n>"; exit 1; }
	@test -n "$(NOTEBOOK)" || { echo "No Group*_Midterm.ipynb found"; exit 1; }
	rm -rf .submission Group$(GROUP)_Midterm.zip
	mkdir .submission
	cp "$(NOTEBOOK)" .submission/Group$(GROUP)_Midterm.ipynb
	cp -r src tools Makefile .submission/
	rm -rf .submission/tools/__pycache__
	cd .submission && zip -qr ../Group$(GROUP)_Midterm.zip Group$(GROUP)_Midterm.ipynb src tools Makefile
	rm -rf .submission
	@echo "Wrote Group$(GROUP)_Midterm.zip"

.PHONY: clean submission
