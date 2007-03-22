operator thevertex alias THEVERTEX pattern op (_, _)
operator mindegree alias MINDEGREE pattern op (_, _)
operator maxdegree alias MAXDEGREE pattern op (_, _)
operator circle alias CIRCLE pattern op (_, _, _)
operator shortestpath alias SHORTESTPATH pattern op (_, _, _ )
operator connectedcomponents alias CONNECTEDCOMPONENTS pattern op ( _ )
operator key alias KEY pattern op ( _ )
operator pos alias POS pattern op ( _ )
operator source alias SOURCE pattern op ( _ )
operator target alias TARGET pattern op ( _ )
operator cost alias COST pattern op ( _ )
operator placenodes alias PLACENODES pattern op ( _ )
operator vertices alias VERTICES pattern op (_)
operator merge alias MERGE pattern op (_, _)
operator partof alias PARTOF pattern _ infixop _
operator equal alias EQUAL pattern _ infixop _
operator constgraph alias CONSTGRAPH pattern _ op [ _, _, fun ] implicit parameter tuple type TUPLE
operator constgraphpoints alias CONSTGRAPHPOINTS pattern _ op [ _, _, fun, _, _ ] implicit parameter tuple type TUPLE