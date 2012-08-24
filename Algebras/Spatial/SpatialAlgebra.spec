operator touches alias TOUCHES pattern _ infixop _
operator adjacent alias ADJACENT pattern _ infixop _
operator overlaps alias OVERLAPS pattern _ infixop _
operator onborder alias ONBORDER pattern _ infixop _
operator ininterior alias ININTERIOR pattern _ infixop _
operator crossings alias CROSSINGS pattern op ( _, _ )
operator single alias SINGLE pattern  op ( _ )
operator distance alias DISTANCE pattern  op ( _, _ )
operator direction alias DIRECTION pattern  op ( _, _, _ )
operator nocomponents alias NOCOMPONENTS pattern  op ( _ )
operator nohalfseg alias NOHALFSEG pattern  op ( _ )
operator size alias SIZE pattern  op ( _ )
operator touchpoints alias TOUCHPOINTS pattern  op ( _, _ )
operator commonborder alias COMMONBORDER pattern  op ( _, _ )
operator bbox alias BBOX pattern  op ( _, _ )
operator insidepathlength alias INSIDEPATHLENGTH pattern  _ infixop _
operator insidescanned alias INSIDESCANNED pattern  _ infixop _
operator insideold alias INSIDEOLD pattern  _ infixop _
operator translate alias TRANSLATE pattern  _ op [list]
operator rotate alias ROTATE pattern  _ op [ _,  _, _ ]
operator clip alias CLIP pattern  op (_, _)
operator windowclippingin alias WINDOWCLIPPINGIN pattern op ( _ , _)
operator windowclippingout alias WINDOWCLIPPINGOUT pattern op ( _ , _)
operator vertices alias VERTICES pattern op ( _ )
operator isempty alias ISEMPTY pattern op(_)
operator = alias EQ pattern _ infixop _
operator # alias NEQ pattern _ infixop _
operator < alias LT pattern _ infixop _
operator intersects alias INTERSECTS pattern _ infixop _
operator inside alias INSIDE pattern _ infixop _
operator intersection alias INTERSECTION pattern op(_,_)
operator minus alias MINUS pattern _ infixop _
operator union alias UNION pattern _ infixop _
operator no_components alias NO_COMPONENTS pattern  op ( _ )
operator no_segments alias NO_SEGMENTS pattern  op ( _ )
operator components alias COMPONENTS pattern op(_)
operator boundary alias BOUNDARY pattern op(_)
operator atpoint alias ATPOINT pattern op(_,_)
operator atposition alias ATPOSITION pattern op(_,_)
operator subline alias SUBLINE pattern op(_,_)
operator + alias PLUS pattern _ infixop _
operator getx alias GETX pattern op(_)
operator gety alias GETY pattern op(_)
operator line2region alias LINE2REGION pattern _ op
operator rect2region alias RECT2REGION pattern _ op
operator area alias AREA pattern op ( _ )
operator polylines alias POLYLINES pattern _ op [ _ ]
operator polylinesC alias POLYLINESC pattern _ op [ _ ]
operator simplify alias SIMPLIFY pattern  op ( _ , _ )
operator segments alias SEGMENTS pattern  op ( _ )
operator get alias GET pattern   op ( _, _ )
operator realminize alias REALMINIZE pattern op ( _ )
operator makeline alias MAKELINE pattern op (_ , _ )
operator makesline alias MAKESLINE pattern op (_ , _ )
operator center alias CENTER pattern op (_)
operator convexhull alias CONVEXHULL pattern op (_)
operator realminize2 alias REALMINIZE2 pattern op(_)
operator commonborder2 alias COMMONBORDER2  pattern _ infixop _
operator collect_line alias COLLECTLINE pattern _ op [ _ ]
operator collect_sline alias COLLECTSLINE pattern _ op [ _ ]
operator collect_points alias COLLECTPOINTS pattern _ op [ _ ]
operator makepoint alias MAKEPOINT pattern op ( _  )
operator get_startsmaller alias GET_STARTSMALLER pattern op ( _ )
operator set_startsmaller alias SET_STARTSMALLER pattern op ( _ , _ )
operator getstartpoint alias GETSTARTPOINT pattern op ( _ )
operator getendpoint alias GETENDPOINT pattern op ( _ )
operator create_sline alias CREATE_SLINE pattern op (_, _)
operator distanceOrthodrome alias DISTANCEORTHODROME pattern op( _, _, _ )
operator create_geoid alias CREATE_GEOID pattern op( _, _, _ )
operator getRadius alias GETRADIUS pattern op( _ )
operator getFlattening alias GETFLATTENING pattern op( _ )
operator point2string alias POINT2STRING pattern op( _ , _ )
operator midpointBetween alias MIDPOINTBETWEEN pattern op( _, _, _, _ )
operator direction2heading alias DIRECTION2HEADING pattern op( _ )
operator heading2direction alias HEADING2DIRECTION pattern op( _ )
operator fromline alias FROMLINE pattern op( _ )
operator gk alias GK pattern op( _, _ )
operator halfSegments alias HALFSEGMENTS pattern op( _ )
operator heading alias HEADING pattern op( _, _, _ )
operator iscycle alias ISCYCLE pattern op( _ )
operator reverseGk alias REVERSEGK pattern op( _ )
operator toline alias TOLINE pattern op( _ )
operator utm alias UTM pattern op( _ )
operator scale alias SCALE pattern _ op [ _, _ ]
operator create_triangle alias CREATE_TRIANGLE pattern op(_, _, _)
operator bufferLine alias BUFFERLINE pattern op(_,_)
operator circle alias CIRCLE pattern op ( _ , _ , _ )
operator longlines alias LONGLINES pattern _ op
operator splitslineatpoints alias SPLITSLINEATPOINTS pattern op ( _ , _ )
operator findCycles alias FINDCYCLES pattern op(_)

