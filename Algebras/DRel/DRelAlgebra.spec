operator createboundary alias CREATEBOUNDARY pattern _ op [_,_,_]
operator getboundaryindex alias GETBOUNDARYINDEX pattern op(_,_)
operator rect2cellgrid alias RECT2CELLGRID pattern _ op [_]

operator drelfdistribute alias DRELFDISTRIBUTE pattern _ op [_,_,_,_,_]
operator dreldistribute alias DRELDISTRIBUTE pattern _ op [_,_,_,_,_]

operator drelpartition alias DRELPARTITION pattern _ op [_]

operator comparedisttype alias COMPAREDISTTYPE pattern _ _ op
operator drelcollect_box alias DRELCOLLECTBOX pattern _ op[_]
operator convert2darray alias CONVERT2DARRAY pattern _ op

operator drelsummarize alias DRELSUMMARIZE pattern _ op

operator drelcreatebtree alias DRELCREATEBTREE pattern _ op[_,_]
operator drelexactmatch alias DRELEXACTMATCH pattern _ _ op[_]
operator drelrange alias DRELRANGE pattern _ _ op[_,_]

operator drelfilter alias DRELFILTER pattern _ op[fun] implicit parameters elem1 types DRELFUNARG1
operator drelproject alias DRELPROJECT pattern _ op [list]
operator drelextend alias DRELEXTEND pattern _ op [funlist] implicit parameters elem1 types DRELFUNARG1
operator drelprojectextend alias DRELPROJECTEXTEND pattern _ op [list;funlist] implicit parameters elem1 types DRELFUNARG1
operator drelhead alias DRELHEAD pattern _ op[_] implicit parameters elem1 types DRELFUNARG1
operator drelrename alias DRELRENAME pattern _ op[_]

operator drellrdup alias DRELLRDUP pattern _ op
operator drellsort alias DRELLSORT pattern _ op
operator drellgroupby alias DRELLGROUPBY pattern _ op[list;funlist] implicit parameters elem1 types DRELRELFUNARG1
operator drellsortby alias DRELPROJECT pattern _ op [list]
