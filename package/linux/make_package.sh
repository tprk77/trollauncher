#!/bin/bash

# Copyright (c) 2020 Tim Perkins

set -o errexit
set -o nounset
set -o pipefail
IFS=$'\n\t'

readonly SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

readonly MESON_FILE="${SCRIPT_DIR}/../../meson.build"

readonly DIST_ID="$(lsb_release -si | tr '[:upper:]' '[:lower:]')"
readonly DIST_REL="$(lsb_release -sr | sed -r -e 's;\.;;g')"
readonly DIST="${DIST_ID}${DIST_REL}"
readonly ARCH="$(dpkg --print-architecture)"

# Search the "meson.build" file for a project version number. Instead of a proper parse, just grep
# the top of the file for it. It won't work for arbitrary input, but that's ok, it will be fine for
# our purposes. Expected format is, e.g., "version : '0.1.0',".
readonly VERSION="$(head -n 20 "${MESON_FILE}" | grep -E "^\s*version : '[^']+',$" \
    | head -n 1 | sed -r -e "s;\s*version : '([^']+)',;\1;")"

if [ -z "${VERSION}" ]; then
    echo "Could not detect project version!" 1>&2
    exit 1
fi

readonly DEB_CONTROL_IN="${SCRIPT_DIR}/debian_control.in"
readonly TROLLAUNCHER="${SCRIPT_DIR}/../../rbuild/trollauncher"
readonly DOT_DESKTOP="${SCRIPT_DIR}/trollauncher.desktop"
readonly ICON_SVG="${SCRIPT_DIR}/../../resource/trollface.svg"
readonly ICON_16="${SCRIPT_DIR}/../../resource/trollface_16.png"
readonly ICON_24="${SCRIPT_DIR}/../../resource/trollface_24.png"
readonly ICON_32="${SCRIPT_DIR}/../../resource/trollface_32.png"
readonly ICON_48="${SCRIPT_DIR}/../../resource/trollface_48.png"
readonly ICON_128="${SCRIPT_DIR}/../../resource/trollface_128.png"

if [ ! -x "${TROLLAUNCHER}" ]; then
    echo "Could not find release build of trollauncher! (Was it built?)" 1>&2
    exit 1
fi

readonly PACKAGE_DIR="${SCRIPT_DIR}/trollauncher_${VERSION}_${DIST}_${ARCH}"

readonly DEB_DIR="${PACKAGE_DIR}/DEBIAN"
readonly BIN_DIR="${PACKAGE_DIR}/usr/bin"
readonly LAUNCHER_DIR="${PACKAGE_DIR}/usr/share/applications"
readonly ICON_DIR="${PACKAGE_DIR}/usr/share/icons/hicolor/scalable/apps"
readonly ICON_16_DIR="${PACKAGE_DIR}/usr/share/icons/hicolor/16x16/apps"
readonly ICON_24_DIR="${PACKAGE_DIR}/usr/share/icons/hicolor/24x24/apps"
readonly ICON_32_DIR="${PACKAGE_DIR}/usr/share/icons/hicolor/32x32/apps"
readonly ICON_48_DIR="${PACKAGE_DIR}/usr/share/icons/hicolor/48x48/apps"
readonly ICON_128_DIR="${PACKAGE_DIR}/usr/share/icons/hicolor/128x128/apps"

# Clear out old stuff
rm -rf "${PACKAGE_DIR}"
rm -f "${PACKAGE_DIR}.deb"

# Create the directory structure
mkdir -p "${DEB_DIR}"
mkdir -p "${BIN_DIR}"
mkdir -p "${LAUNCHER_DIR}"
mkdir -p "${ICON_DIR}"
mkdir -p "${ICON_16_DIR}"
mkdir -p "${ICON_24_DIR}"
mkdir -p "${ICON_32_DIR}"
mkdir -p "${ICON_48_DIR}"
mkdir -p "${ICON_128_DIR}"

# Copy all files
cp "${DEB_CONTROL_IN}" "${DEB_DIR}/control"
cp "${TROLLAUNCHER}" "${BIN_DIR}/trollauncher"
cp "${DOT_DESKTOP}" "${LAUNCHER_DIR}/trollauncher.desktop"
cp "${ICON_SVG}" "${ICON_DIR}/trollface.svg"
cp "${ICON_16}" "${ICON_16_DIR}/trollface.png"
cp "${ICON_24}" "${ICON_24_DIR}/trollface.png"
cp "${ICON_32}" "${ICON_32_DIR}/trollface.png"
cp "${ICON_48}" "${ICON_48_DIR}/trollface.png"
cp "${ICON_128}" "${ICON_128_DIR}/trollface.png"

# Do version substitution
sed -i -r -e "s;@VERSION@;${VERSION};g" -e "s;@ARCH@;${ARCH};g" "${DEB_DIR}/control"

# Make the actual package
dpkg-deb --build "${PACKAGE_DIR}"

exit 0
