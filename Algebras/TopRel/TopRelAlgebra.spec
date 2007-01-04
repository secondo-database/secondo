operator  + alias PLUS pattern _infixop _
operator  - alias MINUS pattern _ infixop _
operator  clustername_of alias CLUSTERNAME_OF pattern op(_,_)
operator  clusterof alias CLUSTEROF pattern op(_,_)
operator  contains alias CONTAINS pattern _ infixop _
operator  createcluster alias CREATECLUSTER pattern op(_,_)
operator  createpgroup alias createpgroup pattern op(_,_)
operator  createprioritypgroup alias CREATEPRIORITYPGROUP pattern op(_,_)
operator  createvalidpgroup alias CREATEVALIDPGROUP pattern op(_,_)
operator  disjoint alias DISJOINT pattern _ infixop _
operator  intersection alias INTERSECTION pattern op(_,_)
operator  invert alias INVERT pattern op(_)
operator  isComplete alias ISCOMPLETE pattern op(_)
operator  multiintersection alias MULTIINTERSECTION pattern op(_,_)
operator  name_of alias NAME_OF pattern op(_)
operator  number_of alias NUMBER_OF pattern op(_)
operator  pwdisjoint alias PWDISJOINT pattern op(_,_)
operator  relax alias RELAX pattern op(_,_)
operator  renamecluster alias RENAMECLUSTER pattern _ op [_]
operator  restrict alias RESTRICT pattern op(_,_)
operator  sizeof alias SIZEOF pattern op(_)
operator  transpose alias TRANSPOSE pattern op(_)
operator  union alias UNION  pattern _ infixop _
operator  unspecified alias UNSPECIFIED pattern op(_)

