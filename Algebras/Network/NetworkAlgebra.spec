operator thenetwork alias THENETWORK pattern op ( _, _ )
operator routes alias ROUTES pattern op ( _ )
operator junctions alias JUNCTIONS pattern op ( _ )
operator sections alias SECTIONS pattern op ( _ )
operator shortest_path alias SHORTEST_PATH pattern op ( _, _ )
operator length alias LENGTH pattern op ( _ )
operator point2gpoint alias POINT2GPOINT pattern op ( _ , _ )
operator distance alias DISTANCE pattern op ( _ , _ )
operator line2gline alias LINE2GLINE pattern op ( _ , _ )
operator = alias EQUAL pattern _ infixop _
operator inside alias INSIDE pattern _ infixop _
operator no_components alias NO_COMPONENTS pattern op ( _ )
operator polygpoints alias POLYGPOINTS pattern op ( _ , _ )
operator routeintervals alias ROUTEINTERVALS pattern op ( _ )
