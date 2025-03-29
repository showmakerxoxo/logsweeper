#!/bin/sh
set -ex

SOURCE_DIR=$(pwd)
BUILD_DIR=${BUILD_DIR:-"./build"}

rm -rf "${BUILD_DIR}"

mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"

cmake "${SOURCE_DIR}"

cmake --build .