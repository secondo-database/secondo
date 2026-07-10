# SECONDO environment detection
#
# Derives the SECONDO build/runtime environment from the tools installed on
# this machine, so that a user only has to provide SECONDO_BUILD_DIR.
#
# Usage:
#   source $SECONDO_BUILD_DIR/CM-Scripts/secondo-detect.sh   # sets & exports the variables
#   $SECONDO_BUILD_DIR/CM-Scripts/secondo-detect.sh          # prints a one-line summary (+ warnings)
#   $SECONDO_BUILD_DIR/CM-Scripts/secondo-detect.sh --check  # same, explicit
#   $SECONDO_BUILD_DIR/CM-Scripts/secondo-detect.sh --print-vars   # emit "NAME=value" lines

secondo_realpath() {
  if command -v realpath >/dev/null 2>&1; then realpath "$1"; return; fi
  if command -v greadlink >/dev/null 2>&1; then greadlink -f "$1"; return; fi
  local p=$1 t
  while [ -L "$p" ]; do
    t=$(readlink "$p")
    case "$t" in /*) p=$t ;; *) p=$(dirname "$p")/$t ;; esac
  done
  ( cd "$(dirname "$p")" 2>/dev/null && printf '%s/%s\n' "$(pwd)" "$(basename "$p")" )
}

# Map a Prolog version number (e.g. 90209) to the JPL binding directory that
# OptServer/OptParser/Jpl build (10/30/70/82). Mirrors makefile.optimizer.
secondo_jplver() {
  local v=${1:-0}
  if   [ "$v" -lt 50200 ] 2>/dev/null; then echo 10
  elif [ "$v" -lt 70000 ] 2>/dev/null; then echo 30
  elif [ "$v" -lt 80200 ] 2>/dev/null; then echo 70
  else echo 82; fi
}

secondo_detect() {
  local os machine cand jc jvmdir opener
  os=$(uname -s); machine=$(uname -m)

  # --- SECONDO_BUILD_DIR (the one thing the user should set) ---------------
  if [ -z "${SECONDO_BUILD_DIR:-}" ]; then
    # Fall back to the tree this script lives in (CM-Scripts/..).
    SECONDO_BUILD_DIR=$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")/.." && pwd)
  fi
  export SECONDO_BUILD_DIR

  # --- platform ------------------------------------------------------------
  if [ -z "${SECONDO_PLATFORM:-}" ]; then
    case "$os" in
      Darwin) SECONDO_PLATFORM=mac_osx ;;
      Linux)
        case "$machine" in
          x86_64|amd64|aarch64|arm64) SECONDO_PLATFORM=linux64 ;;
          *)                          SECONDO_PLATFORM=linux ;;
        esac ;;
      *) SECONDO_PLATFORM=linux64 ;;
    esac
  fi
  export SECONDO_PLATFORM

  # --- Prolog (single source of truth: the swipl binary itself) ------------
  # `swipl --dump-runtime-variables` reports version AND lib dir together, so
  # the JPL binding directory can never drift from the linked libswipl.
  local swipl=${SECONDO_SWIPL:-$(command -v swipl 2>/dev/null || command -v pl 2>/dev/null)}
  SECONDO_PL_FOUND=no
  if [ -n "$swipl" ] && "$swipl" --dump-runtime-variables >/dev/null 2>&1; then
    # sets PLBASE, PLLIBDIR, PLVERSION, PLARCH, PLSOEXT, ... as shell vars
    eval "$("$swipl" --dump-runtime-variables)"
    : "${PL_VERSION:=$PLVERSION}"
    : "${SWI_HOME_DIR:=$PLBASE}"
    : "${PL_LIB_DIR:=$PLLIBDIR}"
    : "${PL_DLL_DIR:=$PLLIBDIR}"
    : "${PL_INCLUDE_DIR:=$PLBASE/include}"
    : "${PL_LIB:=swipl}"
    : "${JPL_JAR:=$PLBASE/lib/jpl.jar}"
    : "${JPL_DLL:=$PLLIBDIR/libjpl.$PLSOEXT}"   # PLSOEXT = so (Linux) / dylib (mac)
    export PL_VERSION SWI_HOME_DIR PL_LIB_DIR PL_DLL_DIR PL_INCLUDE_DIR
    export PL_LIB JPL_JAR JPL_DLL
    SECONDO_PL_FOUND=yes
  fi

  # --- Java (JDK home) -----------------------------------------------------
  if [ -z "${J2SDK_ROOT:-}" ]; then
    if [ "$os" = Darwin ] && [ -x /usr/libexec/java_home ]; then
      J2SDK_ROOT=$(/usr/libexec/java_home 2>/dev/null)   # VERIFY on macOS
    else
      jc=$(command -v javac 2>/dev/null || command -v java 2>/dev/null)
      if [ -n "$jc" ]; then
        jc=$(secondo_realpath "$jc")
        J2SDK_ROOT=$(cd "$(dirname "$jc")/.." && pwd)     # strip /bin/<tool>
      fi
    fi
  fi
  [ -n "${J2SDK_ROOT:-}" ] && export J2SDK_ROOT
  : "${SECONDO_JAVA:=${J2SDK_ROOT:-}/bin/java}"
  export SECONDO_JAVA

  # --- Berkeley DB ---------------------------------------------------------
  if [ -z "${BERKELEY_DB_DIR:-}" ]; then
    if [ "$os" = Darwin ]; then
      # VERIFY on macOS: auto-detect Homebrew -> MacPorts -> common prefixes.
      for cand in \
          "$(brew --prefix berkeley-db 2>/dev/null)" \
          "$(brew --prefix berkeley-db@5 2>/dev/null)" \
          /opt/local /usr/local/opt/berkeley-db /usr/local /usr; do
        if [ -n "$cand" ] && [ -f "$cand/include/db_cxx.h" ]; then
          BERKELEY_DB_DIR=$cand; break
        fi
      done
    else
      for cand in /usr /usr/local; do
        [ -f "$cand/include/db_cxx.h" ] && { BERKELEY_DB_DIR=$cand; break; }
      done
    fi
    : "${BERKELEY_DB_DIR:=/usr}"
  fi
  export BERKELEY_DB_DIR
  : "${BERKELEY_DB_LIB:=db_cxx}"
  export BERKELEY_DB_LIB

  # --- JVM runtime dir (only the optimizer / JPL / JNI need libjvm) ---------
  # libdb_cxx and libswipl resolve on the default loader path; libjvm never
  # does, so this is the single directory the runtime linker actually needs.
  jvmdir=""
  if [ -n "${J2SDK_ROOT:-}" ]; then
    for cand in "$J2SDK_ROOT/lib/server" "$J2SDK_ROOT/jre/lib/server" \
                "$J2SDK_ROOT/jre/lib/amd64/server" "$J2SDK_ROOT/lib"; do
      [ -e "$cand" ] && { jvmdir=$cand; break; }
    done
  fi
  export SECONDO_JVM_LIB_DIR=$jvmdir
  if [ -n "$jvmdir" ]; then
    if [ "$os" = Darwin ]; then
      export DYLD_LIBRARY_PATH="$jvmdir${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"
    else
      export LD_LIBRARY_PATH="$jvmdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    fi
  fi

  # --- misc constants / trivial derivations --------------------------------
  : "${readline:=true}";                                       export readline
  : "${SECONDO_CONFIG:=$SECONDO_BUILD_DIR/bin/SecondoConfig.ini}"; export SECONDO_CONFIG

  # --- PD documentation tooling --------------------------------------------
  : "${PD_HEADER:=$SECONDO_BUILD_DIR/Tools/pd/pd.header}"
  if [ "$os" = Darwin ]; then
    opener=open
  else
    opener=$(command -v xdg-open 2>/dev/null || command -v evince 2>/dev/null \
             || command -v okular 2>/dev/null)
  fi
  : "${PD_DVI_VIEWER:=$opener}"
  : "${PD_PS_VIEWER:=$PD_DVI_VIEWER}"
  export PD_HEADER PD_DVI_VIEWER PD_PS_VIEWER

  # put the pd tools on PATH once
  case ":$PATH:" in
    *":$SECONDO_BUILD_DIR/Tools/pd:"*) ;;
    *) PATH="$PATH:$SECONDO_BUILD_DIR/Tools/pd"; export PATH ;;
  esac
}

# Warn about drift/mismatch cases that used to fail silently.
# Returns non-zero if anything was flagged.
secondo_env_warnings() {
  local w=0 live_swipl live_ver
  if [ "${SECONDO_PL_FOUND:-no}" != yes ]; then
    echo "  ! swipl not found -> optimizer disabled (install swi-prolog, or set SECONDO_SWIPL)"; w=1
  fi
  if [ -z "${J2SDK_ROOT:-}" ] || [ ! -e "${J2SDK_ROOT:-/nonexistent}" ]; then
    echo "  ! JDK not found -> Java GUI / optimizer server will not build (set J2SDK_ROOT)"; w=1
  fi
  if [ ! -f "${BERKELEY_DB_DIR:-/nonexistent}/include/db_cxx.h" ]; then
    echo "  ! db_cxx.h not under ${BERKELEY_DB_DIR:-?} (set BERKELEY_DB_DIR)"; w=1
  fi
  # exported PL_VERSION vs the swipl actually on PATH now (stale shell / upgrade)
  live_swipl=${SECONDO_SWIPL:-$(command -v swipl 2>/dev/null || command -v pl 2>/dev/null)}
  if [ -n "$live_swipl" ] && [ -n "${PL_VERSION:-}" ]; then
    live_ver=$("$live_swipl" --dump-runtime-variables 2>/dev/null \
               | sed -n 's/^PLVERSION="\([0-9]*\)".*/\1/p')
    if [ -n "$live_ver" ] && [ "$live_ver" != "$PL_VERSION" ]; then
      echo "  ! PL_VERSION=$PL_VERSION but $live_swipl reports $live_ver -> re-source your .secondorc"; w=1
    fi
  fi
  return $w
}

# One-line summary (+ warnings). This is what the build prints once per run.
secondo_env_summary() {
  local jplver pl
  jplver=$(secondo_jplver "${PL_VERSION:-0}")
  if [ "${SECONDO_PL_FOUND:-no}" = yes ]; then pl="${PL_VERSION} -> JPL/${jplver}"; else pl="none"; fi
  printf 'secondo env: platform %s | swipl %s | jdk %s | bdb %s\n' \
    "${SECONDO_PLATFORM:-?}" "$pl" "${J2SDK_ROOT:-MISSING}" "${BERKELEY_DB_DIR:-MISSING}"
  secondo_env_warnings
}

# The variables that make/runtime care about, in one place.
SECONDO_VARS='SECONDO_BUILD_DIR SECONDO_PLATFORM PL_VERSION SWI_HOME_DIR PL_LIB_DIR
  PL_DLL_DIR PL_INCLUDE_DIR PL_LIB JPL_JAR JPL_DLL J2SDK_ROOT SECONDO_JAVA
  BERKELEY_DB_DIR BERKELEY_DB_LIB SECONDO_JVM_LIB_DIR SECONDO_CONFIG
  readline PD_HEADER PD_DVI_VIEWER PD_PS_VIEWER'

# Machine-readable dump (NAME=value).
secondo_print_vars() {
  local v
  for v in $SECONDO_VARS; do
    eval "printf '%s=%s\n' \"$v\" \"\${$v:-}\""
  done
}

# Emit GNU-make assignments so makefile.env can load the environment itself,
# with no sourced shell required.
secondo_make_vars() {
  local v val
  echo "# Generated by CM-Scripts/secondo-detect.sh -- do not edit, do not commit."
  for v in $SECONDO_VARS; do
    eval "val=\${$v:-}"
    [ -n "$val" ] && printf '%s ?= %s\n' "$v" "$val"
  done
}

# --- run --------------------------------------------------------------------
secondo_detect

# If executed rather than sourced, act as a CLI.
_secondo_sourced=1
if [ -n "${BASH_SOURCE:-}" ]; then
  [ "${BASH_SOURCE[0]}" = "${0}" ] && _secondo_sourced=0
fi
if [ "$_secondo_sourced" -eq 0 ]; then
  case "${1:-}" in
    ""|--check|--summary) secondo_env_summary ;;
    --print-vars)         secondo_print_vars ;;
    --make-vars)          secondo_make_vars ;;
    *) echo "usage: $0 [--summary|--check|--print-vars|--make-vars]" >&2; exit 2 ;;
  esac
fi
