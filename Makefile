# Makefile

VERBOSE ?=
MESON_FLAGS ?=
NINJA_FLAGS ?= $(if $(VERBOSE),-v)

all: | build
	cd build && ninja $(NINJA_FLAGS)

install: | build
	cd build && ninja $(NINJA_FLAGS) install

build:
	meson $(MESON_FLAGS) build

clean:
	-rm -rf build
