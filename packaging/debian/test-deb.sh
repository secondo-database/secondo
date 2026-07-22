#!/bin/bash
#
# Test an installed SECONDO Debian package.
#
# This script runs INSIDE a pristine Ubuntu container (see docker-test.sh); it
# is not meant to be run on a machine you care about, as it installs the package
# and creates a user. The container has nothing but the package: no build tree,
# no build dependencies. That is the point -- everything the package needs it
# has to bring or to declare.
#
# Usage: test-deb.sh [directory with the .deb files, default /out]
#####################################################################

set -euo pipefail

deb_dir=${1:-/out}
test_user=secondouser
failed=0

check() {
   local name=$1 expected=$2 actual=$3
   if [ "$actual" = "$expected" ]; then
      echo "PASS: $name"
   else
      echo "FAIL: $name (expected '$expected', got '$actual')"
      failed=$((failed + 1))
   fi
}

echo "### Installing the package"
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
# apt (not dpkg -i): it resolves the dependencies of the package against the
# archive, which is what makes this a test of the Depends field.
apt-get install -y -qq "$deb_dir"/secondo_*.deb

check "package installed" "install ok installed" "$(dpkg-query -f '${Status}' -W secondo)"
echo "installed version: $(dpkg-query -f '${Version}' -W secondo)"

echo
echo "### Setting up SECONDO for $test_user"
useradd -m -s /bin/bash "$test_user"

# The tests run as a normal user, not as root: /opt/secondo belongs to root, and
# running them as root would hide exactly the permission problems that the
# optimizer wrapper and the per-user work directory exist to solve.
#
# The heredoc is quoted, so it is the user's shell that expands the variables.
runuser -l "$test_user" -s /bin/bash -c 'bash -s' <<'USER_TESTS' > /tmp/user-tests.log 2>&1
set -euo pipefail

# The installer asks for the database directory and the work directory; the
# empty lines accept the defaults.
printf '\n\n' | /opt/secondo/bin/secondo_installer.sh

# Sourcing must work under "set -e" -- a shell that exits here takes the login
# shell of every user of the package with it.
source ~/.secondorc

echo "PL_VERSION=$PL_VERSION"
echo "JPL_DLL=$JPL_DLL"
[ -f "$JPL_DLL" ] && echo "JPL_DLL_EXISTS=yes" || echo "JPL_DLL_EXISTS=no"
[ -f /opt/secondo/bin/javagui/Javagui.jar ] && echo "JAVAGUI=yes" || echo "JAVAGUI=no"

# The kernel, Berkeley DB and the 'opt' database shipped in the package.
# An int result is displayed as a bare number on a line of its own.
count() { sed -n 's/^[[:space:]]*\([0-9][0-9]*\)[[:space:]]*$/\1/p' | tail -1; }

echo "TTYBDB_COUNT=$(printf "restore database opt from '/opt/secondo/bin/opt';\nquery ten count;\nquit;\n" \
   | SecondoTTYBDB 2>&1 | count)"

# The embedded SWI-Prolog optimizer, now driven directly from the regular TTY:
# a leading "select" (or "sql") is optimized in-process. This is what breaks
# when the optimizer does not match the installed SWI-Prolog.
echo "PLTTY_COUNT=$(printf "open database opt;\nselect count(*) from ten;\nquit;\n" \
   | SecondoTTYBDB 2>&1 | count)"
USER_TESTS

result() { sed -n "s/^$1=//p" /tmp/user-tests.log | tail -1; }

echo
echo "### Results"
check "~/.secondorc sources cleanly under set -e" "yes" \
      "$([ -n "$(result PL_VERSION)" ] && echo yes || echo no)"
check "JPL library exists at the detected path" "yes" "$(result JPL_DLL_EXISTS)"
check "Java GUI is shipped"                     "yes" "$(result JAVAGUI)"
check "SecondoTTYBDB: query ten count"          "10"  "$(result TTYBDB_COUNT)"
check "SecondoTTYBDB: select count(*) from ten (embedded optimizer)" "10" "$(result PLTTY_COUNT)"

if [ "$failed" -ne 0 ]; then
   echo
   echo "### $failed check(s) failed, output of the test user:"
   cat /tmp/user-tests.log
   exit 1
fi

echo
echo "All checks passed."
