# Builds the secondo_native Python extension by wrapping the prebuilt
# SECONDO client-server library (apis/api_cpp/cs/lib/libsecondo.a).
#
# Mirrors the include/link setup of apis/api_cpp/cs/example/makefile so it
# stays consistent with how the reference client is meant to be linked.
#
# Requires the SECONDO build environment (source ~/.secondorc) so that
# SECONDO_BUILD_DIR / SECONDO_PLATFORM / BERKELEY_DB_* are set, and a Python
# with pybind11 available (source the backend .venv).

include $(SECONDO_BUILD_DIR)/makefile.env

CS_DIR   = $(SECONDO_BUILD_DIR)/apis/api_cpp/cs
BDBINCLUDE ?= $(BERKELEY_DB_DIR)/include
BDB_LIB_DIR ?= $(BERKELEY_DB_DIR)/lib
BERKELEY_DB_LIB ?= db_cxx

PYINCLUDES := $(shell python -m pybind11 --includes)
EXTSUFFIX  := $(shell python -c "import sysconfig;print(sysconfig.get_config_var('EXT_SUFFIX'))")

MODULE := secondo_native$(EXTSUFFIX)

INCLUDES = $(PYINCLUDES) -I$(CS_DIR)/include -I$(BDBINCLUDE)

LIBS     = -L$(CS_DIR)/lib -lsecondo -L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB) \
           $(DEFAULTLIB_CXX)

.PHONY: all clean
all: $(MODULE)

# -fvisibility=hidden is what pybind11 asks for: its own types are hidden, so a
# class of ours holding one as a member (SecondoCall) would otherwise be more
# visible than its field and GCC warns about it. PYBIND11_MODULE still exports
# the module init function explicitly.
# The archive is a prerequisite, not just the source: this module is a thin
# wrapper around it, so a kernel rebuild is exactly when it has to be relinked.
# Without it make answers "nothing to be done" and leaves a .so built against
# the previous kernel in place indefinitely -- which does not error, it hangs
# /api/* at the first call.
$(MODULE): secondo_native.cpp $(CS_DIR)/lib/libsecondo.a
	$(CPPC) -O2 -Wall -shared -std=c++17 -fPIC -fvisibility=hidden \
	    $(CCFLAGS) $(INCLUDES) \
	    secondo_native.cpp -o $(MODULE) $(LIBS)

clean:
	rm -f secondo_native*.so
