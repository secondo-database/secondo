#ifndef __OCICPP_H_
#define __OCICPP_H_

/* 
   Main include file should be included 
   by all applications using ocicpp lib 
*/

#define OCICPPLIBVER   0x000506
#define OCICPPLIBMAJOR 0
#define OCICPPLIBMINOR 5
#define OCICPPLIBMICRO 6

#define OCILIBVER	   "8.1"
#define OCILIBVERMAJOR 8
#define OCILIBVERMINOR 1


#include <oci.h>
#include <string>
#include <map>
using namespace std;
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define CELL_NOT_EXISTS   -1
#define CELL_TYPE_UNSUPPORTED -2
#define CELL_UNK_ERROR    -3
#define CELL_OK           1
#define CELL_NULL         2
#define WAS_ERROR(exp) ((exp)!=CELL_OK)

#define DURATION_SESSION	1
#define DURATION_CALL		2

namespace OCICPP {

class CacheMode {
	int cachemode;
public:
	explicit CacheMode(int mode) : cachemode(mode)
	{}
	inline int mode() const { return cachemode; }
	bool operator == (CacheMode cm) { return (cachemode==cm.cachemode); }
	bool operator != (CacheMode cm) { return (cachemode!=cm.cachemode); }
};

const CacheMode CACHE_ON(1);
const CacheMode CACHE_OFF(2);

class LobType {
	int lobtype;
public:
	explicit LobType(int type) : lobtype(type)
	{}
	inline int type() { return lobtype; }	
	bool operator == (LobType lt) { return (lobtype==lt.lobtype); }
	bool operator != (LobType lt) { return (lobtype!=lt.lobtype); }
};

const LobType CLOB(SQLT_CLOB);
const LobType BLOB(SQLT_BLOB);
const LobType BFILE(SQLT_BFILE);

class LobDirection {
	int dir;
public:
	explicit LobDirection(int direction) : dir(direction)
	{}
	inline int direction() const { return dir; }
	bool operator == (LobDirection ld) { return (dir==ld.dir); }
	bool operator != (LobDirection ld) { return (dir!=ld.dir); }
};

const LobDirection LOB_SET(1);
const LobDirection LOB_CUR(2);
const LobDirection LOB_END(3);

class CursorType {
	int cursortype;
public:
	CursorType() : cursortype(0) {}
	explicit CursorType(int type) : cursortype(type)
	{}
	inline int type() const { return cursortype; }
	bool operator == (CursorType ct) { return (cursortype==ct.cursortype); }
	bool operator != (CursorType ct) { return (cursortype!=ct.cursortype); }
};

const CursorType DEFAULT	(0);
const CursorType REFCURSOR	(1);
const CursorType NTABLE		(2);

enum ErrorType {
	ORAERROR, OCICPPERROR
};

} // namespace OCICPP

#define DLEV_FATAL  0
#define DLEV_ERROR  1
#define DLEV_DEBUG  2
#define DLEV_INFO   3
#define DEBUG_LEVEL DLEV_FATAL

#define SERIALIZABLE	OCI_TRANS_SERIALIZABLE
#define READONLY		OCI_TRANS_READONLY





#ifndef __OCICPP_INTERNAL_USE_



#include "db.h"

#include "Connection.h"

#include "Cursor.h"

#include "OraError.h"

#include "Lob.h"

#include "BFile.h"

#include "TLob.h"

#include "RowID.h"

#include "OraType.h"

#include "OraString.h"

#include "OraRaw.h"

#include "OraLabel.h"

#include "OraNumber.h"

#include "OraDummy.h"

#include "OraDate.h"

#include "OraLob.h"

#include "OraBFile.h"

#include "OraRowID.h"

#include "OraRefCur.h"

#else 	
namespace OCICPP {
  void DEBUG(int level,const char *text,...);
  void CHECKERR(OCIError *err,sword status);
  void version(void);
}
#endif /* __OCICPP_INTERNAL_USE_ */

#endif /* __OCICPP_H_ */
