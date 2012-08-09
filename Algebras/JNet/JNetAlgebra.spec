operator createrloc alias CREATERLOC pattern op ( _ , _ , _ )
operator createrint alias CREATERINT pattern op ( _ , _ , _ , _ )
operator createndg alias CREATENDG pattern op ( _ , _ , _ , _ , _ )
operator createjnet alias CREATEJNET pattern op ( _ , _ , _ , _ )
operator createjpoint alias CREATEJPOINT pattern op ( _ , _ )
operator createjpoints alias CREATEJPOINTS pattern op ( _ , _ )
operator createjline alias CREATEJLINE pattern op ( _ , _ )
operator createijpoint alias CREATEIJPOINT pattern op ( _ , _ )
operator createujpoint alias CREATEUJPOINT pattern op ( _ , _ , _ , _ , _ , _ )
operator createmjpoint alias CREATEMJPOINT pattern op ( _ )
operator createstream alias CREATESTREAM pattern op ( _ )
operator createlist alias CREATELIST pattern _ op
operator = alias EQ pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator > alias GT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator < alias LT pattern _ infixop _
operator - alias MINUS pattern _ infixop _
operator # alias NE pattern _ infixop _
operator restrict alias RESTRICT pattern op( _ , _ )
operator tonetwork alias TONETWORK pattern op ( _ , _ )
