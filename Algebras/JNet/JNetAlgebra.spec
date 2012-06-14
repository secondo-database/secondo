operator createjnet alias CREATEJNET pattern op ( _ , _ , _ , _ )
operator createrloc alias CREATERLOC pattern op ( _ , _ , _ )
operator createrint alias CREATERINT pattern op ( _ , _ , _ , _ )
operator createndg alias CREATENDG pattern op ( _ , _ , _ , _ , _ )
operator createstream alias CREATESTREAM pattern op ( _ )
operator createlist alias CREATELIST pattern _ op
operator = alias EQ pattern _ infixop _
operator # alias NE pattern _ infixop _
operator > alias GT pattern _ infixop _
operator < alias LT pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator - alias MINUS pattern _ infixop _
operator restrict alias RESTRICT pattern op( _ , _ )
