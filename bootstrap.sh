#!/bin/bash

# Copyright (c) 2019 Tim Perkins

readonly SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

# Just setup the submodules
cd "${SCRIPT_DIR}"
git submodule update --init --recursive
