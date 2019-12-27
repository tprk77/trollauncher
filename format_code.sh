#!/bin/bash

# Copyright (c) 2019 Tim Perkins

set -o errexit
set -o nounset
set -o pipefail
IFS=$'\n\t'

readonly CPP_FILENAME_REGEX=".*\.[ch]pp$"
readonly CPP_LOCKFILE_REGEX=".*\.#.+\.[ch]pp$"
readonly DIRS='"trollauncher"'

readonly SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

readonly lockfiles="$(cd "${SCRIPT_DIR}" && eval "find ${DIRS} -regex '${CPP_LOCKFILE_REGEX}'")"
if [ -n "${lockfiles}" ]; then
    echo "Emacs Lockfiles detected! Save your files!" 2>&1
    exit 1
fi

(cd "${SCRIPT_DIR}" && eval "find ${DIRS} -regex '${CPP_FILENAME_REGEX}'" | xargs clang-format -i)

exit 0
