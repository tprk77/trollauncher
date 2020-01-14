# Makefile

VERBOSE ?=
MESON_FLAGS ?=
NINJA_FLAGS ?= $(if $(VERBOSE),-v)

all: | build
	cd build && ninja $(NINJA_FLAGS)

release: | rbuild
	cd rbuild && ninja $(NINJA_FLAGS)
	cd rbuild && find . -regex '\./trollauncher\(\.exe\)?' | xargs strip -s

install: | build
	cd build && ninja $(NINJA_FLAGS) install

build:
	meson $(MESON_FLAGS) build

rbuild:
	CXXFLAGS="-Os" meson --buildtype release $(MESON_FLAGS) rbuild

clean:
	-rm -rf build rbuild
