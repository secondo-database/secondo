
operator info alias INFO pattern _ op 

operator loadgraph alias LOADGRAPH pattern _ op 
operator unload alias UNLOAD pattern _ op 
operator clearstat alias CLEARSTAT pattern _ op 

operator cfg alias CFG pattern _ op [_,_]

operator addnodesrel alias ADDNODESREL pattern _ op[_]
operator addedgesrel alias ADDEDGESREL pattern _ op[_,_,_]
operator addindex alias ADDNODEINDEX pattern _ op[_,_]

operator createpgraph alias CREATEPGRAPH pattern op (_)

operator createmempgraph alias CREATEMEMPGRAPH pattern op (_)

operator match1 alias MATCH1 pattern _ _ op[_,_,_]
operator match2 alias MATCH2 pattern _ op[_,_,_,_]
operator match3 alias MATCH3 pattern _ op[_,_]




