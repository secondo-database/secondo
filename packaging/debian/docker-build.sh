#!/bin/bash
#
# Build the SECONDO Debian package in a container.
#
# In contrast to the workflow (which builds on native GitHub runners), this is
# meant for local builds and for reproducing a failing package build. It packs
# the local checkout, not a fresh clone of the upstream repository.
#
# Usage: docker-build.sh [image]
#
#   image   the base image to build in, e.g. ubuntu:24.04 (default) or debian:13.
#####################################################################

set -euo pipefail

image=${1:-ubuntu:24.04}

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
build_dir=$(cd "$script_dir/../.." && pwd)

# The container builds in the mounted checkout as root. The build artifacts are
# handed back to the invoking user afterwards (also when the build fails), so
# that the checkout does not end up with root-owned object files.
docker run --rm -t \
   -e DEBIAN_FRONTEND=noninteractive \
   -e "SECONDO_DEB_BUILD_ID=${SECONDO_DEB_BUILD_ID:-0}" \
   -v "$build_dir:/secondo" \
   -w /secondo \
   "$image" \
   /bin/bash -c "trap 'chown -R $(id -u):$(id -g) /secondo' EXIT; \
                 apt-get update && apt-get install -y git && \
                 git config --global --add safe.directory /secondo && \
                 packaging/debian/build-deb.sh --install-deps"
