


operator createemptytrie alias CREATEEMPTYTREE pattern op(_)
operator insert2trie alias INSERT2TRIE pattern op(_,_,_)
operator searchtrie alias SEARCHTRIE pattern op(_,_)
operator contains alias CONTAINS pattern _ infixop _
operator trieEntries alias trieEntries pattern _ op [_]

operator createInvFile alias CREATEINVFILE pattern _ op [_,_]
operator searchWord alias SEARCHWORD pattern _ op [_]
operator searchPrefix alias SEARCHPREFIX pattern _ op [_]

operator getFileInfo alias GETFILEINFO pattern op(_)

operator wordCount alias WORDCOUNT pattern _ op [_] 
operator prefixCount alias PREFIXCOUNT pattern _ op [_] 

operator defaultInvFileSeparators alias DEFAULTSEPARATORS pattern op()
operator getSeparators alias GETSEPARATORS pattern op(_)

operator containsWord alias CONTAINSWORD pattern op(_,_,_)
operator containsPrefix alias CONTAINSPREFIX pattern op(_,_,_)



