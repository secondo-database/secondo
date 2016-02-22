operator createfbtree alias CREATEFBTREE pattern _op[_,_,_]
operator bulkloadfbtree alias BULKLOADFBTREE pattern _op[_,_,_]
operator removefbtree alias REMOVEFBTREE pattern _op[_,_,_]
operator rebuildfbtree alias REBUILDFBTREE pattern op(_,_)
operator insertfbtree alias INSERTFBTREE pattern _op[_,_,_]
operator deletefbtree alias DELETEFBTREE pattern _op[_,_,_]
operator rebuildfbtree alias REBUILDFBTREE pattern op(_,_)
operator frange alias FRANGE pattern _ op[_,_]
operator fleftrange alias FLEFTRANGE pattern _ op[_]
operator frightrange alias FRIGHTRANGE pattern _ op[_]
operator fexactmatch alias FEXACTMATCH pattern _ op[_]
operator createfrtree alias CREATEFRTREE pattern _op[_,_,_,_]
operator insertfrtree alias INSERTFRTREE pattern _op[_,_,_]
operator deletefrtree alias DELETEFRTREE pattern _op[_,_,_]
operator fwindowintersects alias FWINDOWINTERSECTS pattern op(_,_)
operator rebuildfrtree alias REBUILDFRTREE pattern op(_,_)
operator bulkloadfrtree alias BULKLOADFRTREE pattern _ op[_,_,_,_,_]