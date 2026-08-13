# SECONDO in a container: the server, the production WebUI and the CLI client.
#
# Three stages, because the three things have nothing in common: the frontend
# needs a current Node, the kernel needs the C++/Prolog/JDK toolchain, and the
# runtime needs neither.
#
#   docker build -f packaging/container/Dockerfile -t secondo .
#
# See packaging/container/docker-compose.yml for the usual way to run it.
#
# SECONDO_BUILD_DIR is /opt/secondo in *both* the builder and the runtime
# stage. That is not cosmetic: the kernel reads the variable at runtime
# (SecondoInterfaceTTY.cpp), the embedded optimizer consults its Prolog sources
# under $SECONDO_BUILD_DIR/Optimizer (SecondoPL.cpp), and the backend
# virtualenv contains absolute paths. Building somewhere else and copying to
# /opt/secondo would break all three.


# --- Frontend ---------------------------------------------------------------
# Vite 8 needs Node >= 20.19 and Ubuntu 24.04 ships Node 18, so the bundle is
# built in the official Node image rather than adding a third-party apt repo.
FROM node:24-slim AS frontend

WORKDIR /build/WebUI/frontend
COPY WebUI/frontend/package.json WebUI/frontend/package-lock.json ./
RUN npm ci
COPY WebUI/frontend/ ./
RUN npm run build


# --- Builder ----------------------------------------------------------------
FROM ubuntu:24.04 AS builder

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND=noninteractive

# The build dependencies of the CI workflow (.github/workflows/build.yml), plus
# python3-dev/python3-venv for the pybind11 module of the WebUI bridge and
# pkg-config, which ParallelTransform/makefile uses for the swipl cflags. The
# GitHub runner images have that one preinstalled; a bare ubuntu:24.04 does
# not, and without it the flags are silently empty.
RUN apt-get update && apt-get install -y --no-install-recommends \
       build-essential gcc g++ flex bison libbison-dev libfl-dev pkg-config \
       libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev db5.3-util \
       libjpeg-dev libgsl0-dev libreadline-dev librecode-dev libgmp-dev \
       libgmp10 libncurses-dev libxml2-dev libboost-all-dev libquadmath0 \
       nlohmann-json3-dev default-jdk \
       swi-prolog swi-prolog-nox swi-prolog-java \
       python3 python3-dev python3-venv \
    && rm -rf /var/lib/apt/lists/*

ENV SECONDO_BUILD_DIR=/opt/secondo
WORKDIR /opt/secondo
COPY . /opt/secondo

# makefile.detect derives the platform, the SWI-Prolog paths, the JDK and
# Berkeley DB from the installed tools; check-env fails the build if one of them
# is missing rather than producing a kernel with silently disabled features.
RUN make -f makefile.detect check-env \
    && make -j"$(nproc)" \
    && make -C apis/api_cpp/cs \
    && make -C WebUI venv native

# Unstripped, SecondoBDB alone is ~390 MB of debug info.
RUN strip --strip-unneeded \
       bin/Secondo{BDB,CS,Monitor,Listener,Registrar,Checkpoint} \
       WebUI/backend/native/secondo_native*.so

# The runtime packages, derived from what the binaries actually link instead of
# maintained by hand: the library versions (libgsl.so.27 vs .28, the t64
# renames, the boost soname) follow the base image, and a hand-written list
# would rot the next time it moves.
#
# readlink -f first: ldd reports /lib/x86_64-linux-gnu/... while dpkg knows the
# files under /usr/lib, and dpkg-query does not resolve the usr-merge alias.
# A path nobody owns is tolerated here -- the runtime stage re-runs ldd and
# fails the build if the resulting package set really is incomplete.
RUN ldd bin/Secondo{BDB,CS,Monitor,Listener,Registrar,Checkpoint} \
        WebUI/backend/native/secondo_native*.so \
      | awk '/=> \//{print $3}' | sort -u \
      | xargs -r readlink -f | sort -u > /tmp/linked-libraries.txt \
    && { xargs -r dpkg-query -S < /tmp/linked-libraries.txt 2>/dev/null || true; } \
      | grep -v '^diversion' | cut -d: -f1 | tr ',' '\n' | tr -d ' ' \
      | sort -u > /runtime-packages.txt \
    && cat /runtime-packages.txt


# --- Runtime ----------------------------------------------------------------
FROM ubuntu:24.04

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND=noninteractive

COPY --from=builder /runtime-packages.txt /tmp/runtime-packages.txt

# swi-prolog-nox is explicit: the kernel links libswipl, but the embedded
# optimizer also *consults* the Prolog library at runtime, and no amount of
# ldd reveals that. python3 runs the bridge (the virtualenv links against the
# system interpreter), curl serves the healthcheck.
RUN apt-get update && apt-get install -y --no-install-recommends \
       $(cat /tmp/runtime-packages.txt) \
       swi-prolog-nox db5.3-util python3 curl ca-certificates \
    && rm -rf /var/lib/apt/lists/* /tmp/runtime-packages.txt

# uid 1000 is taken by the "ubuntu" user in this image; the databases are a
# bind-mountable volume, so it is worth having it back for SECONDO.
RUN userdel -r ubuntu 2>/dev/null || true; \
    useradd -m -u 1000 -s /bin/bash secondo && \
    mkdir -p /var/lib/secondo/databases && \
    chown -R secondo:secondo /var/lib/secondo

# --chown on the COPY rather than a later chown -R: recursively changing the
# ownership afterwards would duplicate the whole tree in another layer.
COPY --from=builder --chown=secondo:secondo /opt/secondo/bin /opt/secondo/bin
COPY --from=builder --chown=secondo:secondo /opt/secondo/Optimizer /opt/secondo/Optimizer
COPY --from=builder --chown=secondo:secondo /opt/secondo/Data /opt/secondo/Data
COPY --from=builder --chown=secondo:secondo /opt/secondo/WebUI/backend /opt/secondo/WebUI/backend
COPY --from=frontend --chown=secondo:secondo /build/WebUI/frontend/dist /opt/secondo/WebUI/frontend/dist
COPY --chown=secondo:secondo packaging/container/docker-entrypoint.sh /opt/secondo/bin/

# Fail here, at build time, if the derived package list missed something. Via a
# file rather than a pipe: with pipefail a failing ldd would satisfy a negated
# pipeline, and the check would pass for the wrong reason.
RUN ldd /opt/secondo/bin/Secondo{BDB,CS,Monitor,Listener,Registrar,Checkpoint} \
        /opt/secondo/WebUI/backend/native/secondo_native*.so > /tmp/ldd.txt \
    && ! grep 'not found' /tmp/ldd.txt \
    && rm /tmp/ldd.txt

# This block is also what makes the CLI work: `docker exec` inherits the image
# environment, so `SecondoTTYCS` finds SecondoCS on PATH, has the build dir the
# kernel insists on, and reads the host/port of the server from the config.
ENV SECONDO_BUILD_DIR=/opt/secondo \
    SECONDO_CONFIG=/opt/secondo/bin/SecondoConfig.ini \
    SECONDO_PARAM_SecondoHome=/var/lib/secondo/databases \
    SECONDO_PLATFORM=linux64 \
    PATH=/opt/secondo/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    WEBUI_HOST=0.0.0.0 \
    WEBUI_PORT=8000 \
    SECONDO_SKIP_DB_INIT=false

USER secondo
# A writable working directory: the TTY client keeps its .secondo_history in
# the current directory.
WORKDIR /home/secondo

EXPOSE 1234 8000
VOLUME ["/var/lib/secondo/databases"]

HEALTHCHECK --interval=30s --timeout=5s --start-period=180s --retries=3 \
   CMD curl -fs http://127.0.0.1:8000/api/health || exit 1

ENTRYPOINT ["/opt/secondo/bin/docker-entrypoint.sh"]
