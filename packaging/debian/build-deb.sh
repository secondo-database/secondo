#!/bin/bash
#
# Build the SECONDO Debian package from the local checkout.
#
# Runs on any supported Ubuntu, either directly on the host or inside a
# container (see docker-build.sh). The build environment is detected by
# makefile.detect, the algebras are the default set from
# makefile.algebras.sample, and the package version is derived from
# include/version.h -- nothing needs to be edited to cut a new package.
#
# Usage: build-deb.sh [--install-deps]
#
#   --install-deps   apt-get install the Build-Depends from debian/control
#                    (uses sudo for that; the package itself is built as the
#                    calling user, so do not call this script with sudo)
#
# The resulting packages are placed in packaging/debian/out.
#####################################################################

set -euo pipefail

install_deps=false
case "${1:-}" in
   "")             ;;
   --install-deps) install_deps=true ;;
   *) echo "usage: $0 [--install-deps]" >&2; exit 2 ;;
esac

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
build_dir=$(cd "$script_dir/../.." && pwd)
out_dir="$script_dir/out"

cd "$build_dir"

if [ ! -f include/version.h ]; then
   echo "Error: $build_dir does not look like a SECONDO checkout" >&2
   exit 1
fi

# The package version. Every field is derived, none is maintained by hand:
#
#   4.4.0+build42.git20260714.a1b2c3d4+ubuntu2404
#   ^     ^       ^          ^         ^
#   |     |       |          |         the distribution of the build environment
#   |     |       |          the packaged commit
#   |     |       its commit date
#   |     the build number (the GitHub Actions run number, 0 for local builds)
#   the upstream version from include/version.h
#
# dpkg compares the fields from left to right and the build number numerically,
# so every new build of the workflow supersedes all previous ones. The commit
# id only carries information; it never decides the ordering.
version_field() {
   sed -n "s/^#define SECONDO_VERSION_$1 *\([0-9][0-9]*\).*/\1/p" include/version.h
}
upstream="$(version_field MAJOR).$(version_field MINOR).$(version_field REVISION)"

build_id=${SECONDO_DEB_BUILD_ID:-0}
commit_date=$(git log -1 --date=format:%Y%m%d --format=%cd)
commit_id=$(git rev-parse --short=8 HEAD)
# ubuntu2404, debian13, ... -- whatever we are building on.
distribution=$(. /etc/os-release && echo "${ID}${VERSION_ID//./}")

deb_version="${upstream}+build${build_id}.git${commit_date}.${commit_id}+${distribution}"

echo "Building SECONDO $deb_version"

# The packaging metadata is copied (not linked) into the build tree, because
# the changelog stanza below is generated and must not touch the checkout.
rm -rf "$build_dir/debian"
cp -a "$script_dir/debian" "$build_dir/debian"

{
   printf 'secondo (%s) stable; urgency=low\n\n' "$deb_version"
   printf '  * Automated build of commit %s\n\n' "$(git rev-parse HEAD)"
   printf ' -- %s  %s\n\n' \
      "$(sed -n 's/^Maintainer: //p' debian/control)" "$(date -R)"
   cat "$script_dir/debian/changelog"
} > "$build_dir/debian/changelog"

if [ "$install_deps" = true ]; then
   # Only the dependencies need root. The package itself is built as the calling
   # user (dpkg-buildpackage fakes the root it needs), so that this script can be
   # called without sudo -- sudo does not reliably pass the environment through
   # (SECONDO_DEB_BUILD_ID was silently lost on Ubuntu 26.04).
   sudo=""
   [ "$(id -u)" -eq 0 ] || sudo="sudo"

   export DEBIAN_FRONTEND=noninteractive
   $sudo apt-get update
   # The toolchain, plus the Build-Depends of debian/control -- which is the
   # single source of truth for the build dependencies.
   $sudo apt-get install -y build-essential debhelper fakeroot dpkg-dev
   $sudo apt-get build-dep -y ./
fi

dpkg-buildpackage -us -uc -b -rfakeroot

mkdir -p "$out_dir"
mv -v "$build_dir"/../secondo*"$deb_version"* "$out_dir"

echo
echo "The packages have been written to $out_dir"
