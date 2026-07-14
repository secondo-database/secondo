#!/bin/bash
#
# Install and test a built SECONDO Debian package in a pristine container.
#
# The container gets the package and nothing else: no build tree, no build
# dependencies. Everything the package needs it has to bring itself or to
# declare in its Depends.
#
# Usage: docker-test.sh [image]
#
#   image   the base image to test in, e.g. ubuntu:24.04 (default) or debian:13.
#           Test the package in the image it was built in.
#####################################################################

set -euo pipefail

image=${1:-ubuntu:24.04}

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
out_dir="$script_dir/out"

if ! ls "$out_dir"/secondo_*.deb >/dev/null 2>&1; then
   echo "Error: no package in $out_dir. Run build-deb.sh or docker-build.sh first." >&2
   exit 1
fi

docker run --rm -t \
   -v "$out_dir:/out:ro" \
   -v "$script_dir/test-deb.sh:/test-deb.sh:ro" \
   "$image" \
   /test-deb.sh /out
