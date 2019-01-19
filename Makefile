SHELL ?= /bin/sh
CXX ?= g++
MAKE ?= make
RM ?= rm

CXXFLAGS := -O2 -pipe -g -Wall -Wextra -std=c++14 $(CXXFLAGS)
LIBS := uboostfs/filesystem.a $(LIBS)
OBJS := btrcompressor.o

all: btrcompressor

uboostfs/filesystem.a:
	cd uboostfs && $(MAKE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

btrcompressor: $(OBJS) $(LIBS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

clean:
	cd uboostfs && $(MAKE) clean
	$(RM) $(OBJS) btrcompressor

.PHONY: all clean
