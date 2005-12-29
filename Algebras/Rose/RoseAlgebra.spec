operator = alias EQUAL pattern _ infixop _
operator # alias UNEQUAL pattern _ infixop _
operator disjoint alias DISJOINT pattern _ infixop _
operator inside alias INSIDE pattern _ infixop _
operator area_disjoint alias AREA_DISJOINT pattern _ infixop _ 
operator edge_disjoint alias EDGE_DISJOINT pattern _ infixop _ 
operator edge_inside alias EDGE_INSIDE pattern  _ infixop _ 
operator vertex_inside alias VERTX_INSIDE pattern _ infixop _
operator intersects alias INTERSECTS pattern _ infixop _
operator meets alias MEETS pattern _ infixop _
operator border_in_common alias BORDER_IN_COMMON pattern _ infixop _
operator adjacent alias ADJACENT pattern _ infixop _
operator encloses alias ENCLOSES pattern _ infixop _
operator intersection alias INTERSECTION pattern op (_, _)
operator plus alias PLUS pattern _ infixop _
operator minus alias MINUS pattern _ infixop _
operator common_border alias COMMON_BORDER pattern op (_, _)
operator on_border_of alias ON_BORDER_OF pattern _ infixop _
operator vertices alias VERTICES pattern op (_)
operator interior alias INTERIOR pattern op (_)
operator contour alias CONTOUR pattern op (_)
operator no_of_components alias NO_OF_COMPONENTS pattern op (_)
operator dist alias DIST pattern op (_, _)
operator diameter alias DIAMETER pattern op (_)
operator length alias LENGTH pattern op (_)
operator area alias AREA pattern op (_)
operator perimeter alias PERIMETER pattern op (_)
operator setDeviationValue alias SETDEVIATIONVALUE pattern op (_)
operator chooseTriangulator alias CHOOSETRIANGULATOR pattern op (_)