operator createboundary alias CREATEBOUNDARY pattern _ op [_,_,_]
operator getboundaryindex alias GETBOUNDARYINDEX pattern op(_,_)
operator rect2cellgrid alias RECT2CELLGRID pattern _ op [_]

operator drelfdistribute alias DRELFDISTRIBUTE pattern _ op [_,_,_,_,_]
operator dreldistribute alias DRELDISTRIBUTE pattern _ op [_,_,_,_,_]

operator drelpartition alias DRELPARTITION pattern _ op [_,_]

operator comparedisttype alias COMPAREDISTTYPE pattern _ _ op
operator drelcollect_box alias DRELCOLLECTBOX pattern _ op[_]
operator drelconvert alias DRELCONVERT pattern _ op

operator dsummarize alias DSUMMARIZE pattern _ op

operator drelcreatebtree alias DRELCREATEBTREE pattern _ op[_,_]
operator drelexactmatch alias DRELEXACTMATCH pattern _ _ op[_]
operator drelrange alias DRELRANGE pattern _ _ op[_,_]

operator drelbulkloadrtree alias DRELBULKLOADRTREE pattern _ op[_,_]
operator drelwindowintersects alias DRELWINDOWINTERSECTS pattern _ _ op[_]

operator drelfilter alias DRELFILTER pattern _ op[fun] implicit parameters elem1 types DRELFUNARG1
operator drelextend alias DRELEXTEND pattern _ op [funlist] implicit parameters elem1 types DRELFUNARG1
operator drelprojectextend alias DRELPROJECTEXTEND pattern _ op [list;funlist] implicit parameters elem1 types DRELFUNARG1
operator project alias PROJECT pattern _ op [list]
operator head alias HEAD pattern _ op[_]
operator rename alias RENAME pattern _ op[_]

operator lrdup alias LRDUP pattern _ op
operator lsort alias LSORT pattern _ op
operator lsortby alias LSORTBY pattern _ op [list]
operator lgroupby alias DRELLGROUPBY pattern _ op[list;funlist] implicit parameters elem1 types DRELRELFUNARG1

operator rdup alias RDUP pattern _ op
operator sort alias LSORT pattern _ op
operator sortby alias SORTBY pattern _ op [list]
operator drelgroupby alias DRELGROUPBY pattern _ op[list;funlist] implicit parameters elem1 types DRELRELFUNARG1

operator drelsortmergejoin alias DRELSORTMERGEJOIN pattern _ _ op [_,_]
operator drelitHashJoin alias DRELITHASHJOIN pattern _ _ op [_,_]