# SECONDO Debian packages

This directory contains everything needed to build the Debian packages of the
[SECONDO](https://secondo-database.github.io/) database for the supported Ubuntu
releases (22.04, 24.04, 26.04).

```
debian/         the packaging metadata (one copy, used for every Ubuntu release)
build-deb.sh    builds the package from the local checkout
docker-build.sh builds the package for a given Ubuntu release in a container
tools/          the wrapper scripts that are shipped in /opt/secondo/bin
repository/     the apt repository index that is published on GitHub Pages
```

There is nothing to configure: the build environment (platform, SWI-Prolog, JDK,
Berkeley DB) is detected by `CM-Scripts/secondo-detect.sh`, the algebras are the
default set of `makefile.algebras.sample`, and the package version is derived from
`include/version.h` and the packaged commit.

## Installing a package

The workflow below publishes an apt repository at
<https://secondo-database.github.io/secondo/apt/>:

```bash
echo 'deb [trusted=yes] https://secondo-database.github.io/secondo/apt/ubuntu/24.04/ ./' \
  | sudo tee /etc/apt/sources.list.d/secondo.list
sudo apt-get update
sudo apt-get install secondo
```

## Building the packages

### With GitHub Actions (the usual way)

Run the *Build Debian packages* workflow manually (`workflow_dispatch`). It builds
the package on a native runner for every supported Ubuntu release, attaches the
`.deb` files to the workflow run and — unless `publish` is unchecked — pushes them
to the apt repository on the `gh-pages` branch.

Every run produces a higher package version than the run before it, so a published
package always supersedes the previously published one:

```
secondo_4.4.0+build42.git20260714.a1b2c3d4+ubuntu2404_amd64.deb
              ^^^^^^^ the run number of the workflow
```

### In a container

```bash
./docker-build.sh ubuntu:24.04
```

Builds *your* working tree (not a fresh clone) in the given image and writes the
packages to `packaging/debian/out`. The distribution of the image ends up in the
package version (`+ubuntu2404`, `+debian13`, …), so the image is the only thing
that has to change to package for another distribution.

### On the local machine

On an Ubuntu machine, from the checkout:

```bash
packaging/debian/build-deb.sh --install-deps
```

Without `--install-deps`, the build dependencies (the `Build-Depends` of
`debian/control`) are expected to be installed already, and root is not needed.

## Testing the packages

```bash
./docker-test.sh ubuntu:24.04
```

Installs the package from `packaging/debian/out` in a pristine container and runs
SECONDO in it as a normal user: it runs `secondo_installer.sh`, sources the
generated `~/.secondorc`, and queries the shipped `opt` database through
`SecondoTTYBDB` — both an executable-plan query and an SQL-dialect query, the
latter optimized in-process by the embedded optimizer.

The container has nothing but the package — no build tree, no build dependencies.
Everything SECONDO needs the package has to bring itself or to declare in its
`Depends`, and the package is installed with `apt` so those dependencies are
really resolved against the archive.

The workflow runs this for every release it builds, and refuses to publish a
package that fails.
