"""
This dictionary contains all error codes and descriptions of errors for Secondo server.
"""
secondo_errors = {

0: "ERR_NO_ERROR",
1: "ERR_CMD_NOT_RECOGNIZED",
2: "ERR_IN_QUERY_EXPR",
3: "ERR_EXPR_NOT_EVALUABLE",
4: "ERR_NO_OBJ_CREATED",
5: "ERR_NO_TYPE_DEFINED",
6: "ERR_NO_DATABASE_OPEN",
7: "ERR_DATABASE_OPEN",
701: "ERR_CREATE_DATABASE",
702: "ERR_DELETE_DATABASE",
8: "ERR_UNDEF_OBJ_VALUE",
9: "ERR_SYNTAX_ERROR",

10: "ERR_IDENT_USED",
11: "ERR_IDENT_UNKNOWN_TYPE",
12: "ERR_IDENT_UNKNOWN_OBJ",
13: "ERR_EXPR_TYPE_NEQ_OBJ_TYPE",
14: "ERR_TYPE_NAME_USED_BY_OBJ",
15: "ERR_IDENT_RESERVED",
16: "ERR_UPDATE_FOR_DERIVED_OBJ_UNSUPPORTED",

20: "ERR_TRANSACTION_ACTIVE",
21: "ERR_NO_TRANSACTION_ACTIVE",
22: "ERR_BEGIN_TRANSACTION_FAILED",
23: "ERR_COMMIT_OR_ABORT_FAILED",
24: "ERR_IN_DEFINITIONS_FILE",
25: "ERR_IDENT_UNKNOWN_DB_NAME",
26: "ERR_PROBLEM_IN_WRITING_TO_FILE",
27: "ERR_DB_NAME_NEQ_IDENT",
28: "ERR_PROBLEM_IN_READING_FILE",
29: "ERR_IN_LIST_STRUCTURE_IN_FILE",

30: "ERR_CMD_NOT_YET_IMPL",
31: "ERR_CMD_LEVEL_NOT_YET_IMPL",
32: "ERR_CMD_NOT_IMPL_AT_THIS_LEVEL",



40: "ERR_IN_TYPE_DEFINITION",
41: "ERR_NAME_DOUBLY_DEFINED",
42: "ERR_IN_TYPE_EXPRESSION",
 
50: "ERR_IN_OBJ_DEFINITION",
51: "ERR_OBJ_NAME_DOUBLY_DEFINED",
52: "ERR_WRONG_TYPE_EXPR_FOR_OBJ",
53: "ERR_WRONG_LIST_REP_FOR_OBJ",

60: "ERR_KIND_DOES_NOT_MATCH_TYPE_EXPR",
61: "ERR_SPECIFIC_KIND_CHECKING_ERROR",

70: "ERR_IN_VALUELIST_TC_V",
71: "ERR_SPECIFIC_FOR_TYPE_CONSTRUCTOR",
72: "ERR_IN_VALUELIST_TC",
73: "ERR_AT_POS_IN_VALUELIST",

80: "ERR_IN_SECONDO_PROTOCOL",
81: "ERR_CONNECTION_TO_SERVER_LOST",
82: "ERR_IDENT_UNKNOWN_DB_OBJECT",
83: "ERR_OBJ_NAME_IN_FILE_NEQ_IDENT",
84: "ERR_IDENT_ALREADY_KNOWN_IN_DB",
85: "ERR_ALGEBRA_UNKNOWN",
86: "ERR_IN_FILETRANSFER",
87: "ERR_INVALID_FILE_NAME",
88: "ERR_FILE_EXISTS",
89: "ERR_FILE_NOT_EXISTS",

100: "ERR_SYSTEM_ERROR",
101: "ERR_SYSTEM_DIED",

-1: "ERR_UNKNOWN_RETURN_CODE",


-999: "E_SMI_BDB",

-1001: "E_SMI_STARTUP",
-1002: "E_SMI_SHUTDOWN",
-1003: "E_SMI_DB_CREATE",
-1004: "E_SMI_DB_OPEN",
-1005: "E_SMI_DB_CLOSE",
-1006: "E_SMI_DB_ERASE",
-1007: "E_SMI_DB_NOTOPEN",
-1008: "E_SMI_DB_NOTCLOSED",
-1009: "E_SMI_DB_INVALIDNAME",
-10010: "E_SMI_DB_EXISTING",
-10011: "E_SMI_DB_NOTEXISTING",
-10012: "E_SMI_DB_LOOKUP",
-10013: "E_SMI_DB_INSERT_CATALOG",
-10014: "E_SMI_DB_DELETE_CATALOG",
-10015: "E_SMI_DB_UPDATE_CATALOG",
-10016: "E_SMI_DB_NOTFOUND",
-10017: "E_SMI_DB_NOTLOCKED",
-10018: "E_SMI_DB_REGISTER",
-10019: "E_SMI_DB_UNREGISTER",
-10020: "E_SMI_DB_LOCK",
-10021: "E_SMI_DB_UNLOCK",
-10022: "E_SMI_DB_LOCK_DEADLOCK",

-1051: "E_SMI_TXN_BEGIN",
-1052: "E_SMI_TXN_COMMIT",
-1053: "E_SMI_TXN_ABORT",
-1054: "E_SMI_TXN_RUNNING",
-1055: "E_SMI_TXN_NOTRUNNING",

-1101: "E_SMI_CATALOG_LOOKUP",
-1102: "E_SMI_CATALOG_INSERT",
-1103: "E_SMI_CATALOG_DELETE",
-1104: "E_SMI_CATALOG_KEYEXIST",
-1105: "E_SMI_CATALOG_NOTFOUND",
-1106: "E_SMI_CATALOG_RENAME",

-1201: "E_SMI_FILE_INVALIDNAME",
-1202: "E_SMI_FILE_NOFILEID",
-1203: "E_SMI_FILE_BADCONTEXT",
-1204: "E_SMI_FILE_CREATE",
-1205: "E_SMI_FILE_OPEN",
-1206: "E_SMI_FILE_CLOSE",
-1207: "E_SMI_FILE_KEYEXIST",
-1208: "E_SMI_FILE_ISTEMP",

-1251: "E_SMI_RECORD_NOTINIT",
-1252: "E_SMI_RECORD_READ",
-1253: "E_SMI_RECORD_WRITE",
-1254: "E_SMI_RECORD_SELECT",
-1255: "E_SMI_RECORD_APPEND",
-1256: "E_SMI_RECORD_DELETE",
-1257: "E_SMI_RECORD_SELECTALL",
-1258: "E_SMI_RECORD_INSERT",
-1259: "E_SMI_RECORD_TRUNCATE",
-1260: "E_SMI_RECORD_READONLY",
-1261: "E_SMI_RECORD_FINISH",

-1271: "E_SMI_CURSOR_NOTOPEN",
-1272: "E_SMI_CURSOR_NEXT",
-1273: "E_SMI_CURSOR_ENDOFSCAN",
-1274: "E_SMI_CURSOR_DELETE",
-1275: "E_SMI_CURSOR_FINISH",

-1290: "E_SMI_PREFETCH_RANGE"
}



class GeneralError(Exception):
    """
    This exception class is the base class of all other error exceptions,
    used to catch all errors with one single except statement.
    """
    def __init__(self, error, *args):
        self.error = error
        super(GeneralError, self).__init__(error, *args)


class SecondoAPI_Error(GeneralError):
    """
    The class that is responsible for raising Exception for errors that are related
    to the PySecondo interface rather than the database itself.
    """
    def __init__(self, error, *args):
        self.error = error
        super(SecondoAPI_Error, self).__init__(error, *args)


class SecondoError(GeneralError):
    """
    The class that is responsible for raising Exception for errors that are related to the database.
    """
    def __init__(self, error, *args):
        self.error = error
        super(SecondoError, self).__init__(error, *args)


class UnexpectedSystemError(SecondoError):
    """
    The class that is responsible for raising Exception for errors that are related to the database's operation
    e.g. an unexpected disconnect, the data source name not found,
    a transaction could not be processed, a memory allocation error, etc.
    """
    def __init__(self, error, *args):
        self.error = error
        super(UnexpectedSystemError, self).__init__(error, *args)

