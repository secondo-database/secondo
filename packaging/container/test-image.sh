#!/bin/bash
#
# Test a built SECONDO container image.
#
# Starts the image, waits for the stack to come up and drives it the way a user
# would: through the WebUI's REST interface and through SecondoTTYCS inside the
# container. Nothing of the checkout is used at runtime -- everything the image
# needs it has to bring itself.
#
# Usage: test-image.sh [image, default "secondo"]
#####################################################################

set -uo pipefail

image=${1:-secondo}
container="secondo-test-$$"
failed=0

# The WebUI is published on a free high port so that a SECONDO running on the
# host does not collide with the test.
port=18000

cleanup() {
   docker rm -f "$container" > /dev/null 2>&1
   docker rm -f "$container-skip" > /dev/null 2>&1
}
trap cleanup EXIT

check() {
   local name=$1 expected=$2 actual=$3
   if [[ "$actual" == *"$expected"* ]]; then
      echo "PASS: $name"
   else
      echo "FAIL: $name"
      echo "      expected to contain: $expected"
      echo "      got: ${actual:0:500}"
      failed=$((failed + 1))
   fi
}

echo "### Starting $image"
docker run -d --name "$container" -p "127.0.0.1:$port:8000" "$image" > /dev/null

# The first start restores berlintest and opt, which dominates the wait.
echo -n "waiting for the WebUI "
for i in $(seq 1 300); do
   if curl -fs "http://127.0.0.1:$port/api/health" > /dev/null 2>&1; then break; fi
   if ! docker ps --format '{{.Names}}' | grep -qx "$container"; then
      echo; echo "The container exited:"; docker logs "$container"; exit 1
   fi
   if [ "$i" = 300 ]; then
      echo; echo "The WebUI did not come up:"; docker logs --tail 100 "$container"; exit 1
   fi
   echo -n .
   sleep 2
done
echo " up"

echo
echo "### WebUI"
check "health endpoint"   '"status"'     "$(curl -fs http://127.0.0.1:$port/api/health)"
check "production bundle" 'id="root"'    "$(curl -fs http://127.0.0.1:$port/)"

# The first SECONDO command races the monitor's per-connection server fork.
dbs=""
for i in $(seq 1 30); do
   dbs=$(curl -fs "http://127.0.0.1:$port/api/databases" || true)
   [[ "$dbs" == *BERLINTEST* ]] && break
   sleep 1
done
check "berlintest via REST" 'BERLINTEST' "$dbs"
check "opt via REST"        'OPT'        "$dbs"

echo
echo "### The command line inside the container"
check "spatial query" 'point: (9396,9871)' \
   "$(printf 'open database berlintest;\nquery mehringdamm;\nquit;\n' \
      | docker exec -i "$container" SecondoTTYCS 2>&1)"

# Also exercises the embedded optimizer in the forked server process, which is
# the path that needs $SECONDO_BUILD_DIR/Optimizer to be present and writable.
check "SQL via the embedded optimizer" 'Optimized plan: query ten  count' \
   "$(printf 'open database opt;\nselect count(*) from ten;\nquit;\n' \
      | docker exec -i "$container" SecondoTTYCS 2>&1)"

echo
echo "### The preloaded databases"
check "restored into the database directory" 'BERLINTEST' \
   "$(docker exec "$container" ls /var/lib/secondo/databases)"

echo
echo "### SECONDO_SKIP_DB_INIT"
docker run -d --name "$container-skip" -e SECONDO_SKIP_DB_INIT=true "$image" > /dev/null
for i in $(seq 1 60); do
   docker logs "$container-skip" 2>&1 | grep -q 'not preloading' && break
   sleep 1
done
check "the restore is skipped" 'not preloading' "$(docker logs "$container-skip" 2>&1)"

echo
if [ "$failed" -ne 0 ]; then
   echo "### $failed check(s) failed, container log:"
   docker logs --tail 100 "$container"
   exit 1
fi

echo "All checks passed."
