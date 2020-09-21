# Makefile

SHELL := bash
.SHELLFLAGS := -o errexit -o nounset -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules
ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error Please use a version of Make supporting .RECIPEPREFIX)
endif
.RECIPEPREFIX = >

MESON := meson
NINJA := ninja -v

# Automatically collect all sources
TL_SRC_DIRS := trollauncher
TL_SRCS := $(shell find $(TL_SRC_DIRS) -type f -regex ".*\.[ch]pp$$")

all: build/build.sentinel

build:
> $(MESON) setup build

build/build.sentinel: $(TL_SRCS) | build
> $(NINJA) -C build
> touch build/build.sentinel

release: rbuild/build.sentinel

rbuild:
> $(MESON) setup --buildtype minsize rbuild

rbuild/build.sentinel: $(TL_SRCS) | rbuild
> $(NINJA) -C rbuild
> find rbuild -regex '.+/trollauncher\(\.exe\)?' | xargs strip -s
> touch rbuild/build.sentinel

clean:
> -$(RM) -rf build rbuild

# This will really only work on Ubuntu 18.04, oh well
DEV_DEPENDS := build-essential pkg-config python3 python3-pip python3-setuptools python3-wheel \
    ninja-build g++-8 libboost-all-dev libzip-dev libwxgtk3.0-dev libprocps-dev

devdeps:
> sudo apt-get install $(DEV_DEPENDS)
> sudo -H pip3 install meson

.PHONY: all release clean devdeps
