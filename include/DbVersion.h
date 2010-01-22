/*

Some macros checking for certain berkeley db versions


*/
#ifndef DbVersion_H
#define DbVersion_H

#include <db_cxx.h>

#undef DB_VERSION_REQUIRED
#define DB_VERSION_REQUIRED(major, minor) ((DB_VERSION_MAJOR > major) || \
                                           (DB_VERSION_MAJOR == major && \
                                            DB_VERSION_MINOR >= minor))


#endif
