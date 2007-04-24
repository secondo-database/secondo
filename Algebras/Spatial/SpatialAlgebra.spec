operator touches alias TOUCHES pattern _ infixop _
operator adjacent alias ADJACENT pattern _ infixop _
operator overlaps alias OVERLAPS pattern _ infixop _
operator onborder alias ONBORDER pattern _ infixop _
operator ininterior alias ININTERIOR pattern _ infixop _
operator crossings alias CROSSINGS pattern op ( _, _ )
operator single alias SINGLE pattern  op ( _ )
operator distance alias DISTANCE pattern  op ( _, _ )
operator direction alias DIRECTION pattern  op ( _, _ )
operator nocomponents alias NOCOMPONENTS pattern  op ( _ )
operator nohalfseg alias NOHALFSEG pattern  op ( _ )
operator size alias SIZE pattern  op ( _ )
operator touchpoints alias TOUCHPOINTS pattern  op ( _, _ )
operator commonborder alias COMMONBORDER pattern  op ( _, _ )
operator bbox alias BBOX pattern  op ( _ )
operator insidepathlength alias INSIDEPATHLENGTH pattern  _ infixop _
operator insidescanned alias INSIDESCANNED pattern  _ infixop _
operator insideold alias INSIDEOLD pattern  _ infixop _
operator translate alias TRANSLATE pattern  _ op [list]
operator clip alias CLIP pattern  op (_, _)
operator windowclippingin alias WINDOWCLIPPINGIN pattern op ( _ , _)
operator windowclippingout alias WINDOWCLIPPINGOUT pattern op ( _ , _)
operator vertices alias VERTICES pattern op ( _ )
operator isempty alias ISEMPTY pattern op(_)
operator = alias EQ pattern _ infixop _
operator # alias NEQ pattern _ infixop _
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
operator simplify alias SIMPLIFY pattern  op ( _ , _ )






