
operator set_histogram1d alias SET_HISTOGRAM1D pattern _ op
operator set_histogram2d alias SET_HISTOGRAM2D pattern _ _ op

operator create_histogram1d alias CREATE_HISTOGRAM1D pattern _ op [_,_]
operator create_histogram2d alias CREATE_HISTOGRAM2D pattern _ op [_,_,_]

operator create_histogram1d_equiwidth alias CREATE_HISTOGRAM1D_EQUIWIDTH pattern _ op[_,_]
operator create_histogram2d_equiwidth alias CREATE_HISTOGRAM2D_EQUIWIDTH pattern _ op[_,_,_,_]

operator create_histogram1d_equicount alias CREATE_HISTOGRAM1D_EQUICOUNT pattern _ op[_,_]
operator create_histogram2d_equicount alias CREATE_HISTOGRAM2D_EQUICOUNT pattern _ op[_,_,_,_]

operator no_components alias NO_COMPONENTS pattern op(_)

operator binsX alias BINSX pattern op(_)
operator binsY alias BINSY pattern op(_)

operator binrange_min alias BINRANGE_MIN pattern op(_,_)
operator binrange_max alias BINRANGE_MAX pattern op(_,_)
operator binrange_minX alias BINRANGE_MINX pattern op(_,_)
operator binrange_maxX alias BINRANGE_MAXX pattern op(_,_)
operator binrange_minY alias BINRANGE_MINY pattern op(_,_)
operator binrange_maxY alias BINRANGE_MAXY pattern op(_,_)

operator getcount1d alias GETCOUNT1D pattern op(_,_)
operator getcount2d alias GETCOUNT2D pattern op(_,_,_)

operator findbin alias FINDBIN pattern op(_,_)
operator findbinX alias FINDBINX pattern op(_,_)
operator findbinY alias FINDBINY pattern op(_,_)

operator is_refinement alias IS_REFINEMENT pattern op(_,_)

operator < alias LT pattern _ infixop _
operator = alias EQUAL pattern _ infixop _

operator find_minbin alias FIND_MINBIN pattern _ op
operator find_maxbin alias FIND_MAXBIN pattern _ op

operator mean alias MEAN pattern _ op
operator meanX alias MEANX pattern _ op
operator meanY alias MEANY pattern _ op

operator variance alias VARIANCE pattern _ op
operator varianceX alias VARIANCEX pattern _ op
operator varianceY alias VARIANCEY pattern _ op
operator covariance alias COVARIANCE pattern _ op

operator distance alias DISTANCE pattern op(_,_)

operator translatehistogram alias TRANSLATEHISTOGRAM pattern _ op[_]

operator usehistogram alias USEHISTOGRAM pattern _ op[ list; fun ]
    implicit parameter element type ELEMENT
operator usehistogram2 alias USEHISTOGRAM2 pattern _ _ op[ list; fun ]
    implicit parameters element1, element2 types ELEMENT, ELEMENT

operator fold alias FOLD pattern _ op [ _; _ ]

operator shrink_eager alias SHRINK_EAGER pattern op(_, _, _)
operator shrink_eager2 alias SHRINK_EAGER2 pattern op(_, _, _, _, _)

operator shrink_lazy alias SHRINK_LAZY pattern op(_, _, _)
operator shrink_lazy2 alias SHRINK_LAZY2 pattern op(_, _, _, _, _)

operator insert1dvalue alias INSERT1DVALUE pattern op(_, _, _)
operator insert1d alias INSERT1D pattern op(_, _)
operator insert2dvalue alias INSERT2DVALUE pattern op(_, _, _, _)
operator insert2d alias INSERT2DVALUE pattern op(_, _, _)

