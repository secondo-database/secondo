operator + alias PLUS pattern _ infixop _
operator - alias MINUS pattern _ infixop _
operator * alias TIMES pattern _ infixop _
operator / alias DIVIDEDBY pattern _ infixop _

operator div alias DIV pattern _ infixop _
operator mod alias MOD pattern _ infixop _
operator randint alias RANDINT pattern op ( _ )
operator log alias LOG pattern op ( _ )

operator > alias GT pattern _ infixop _
operator < alias LT pattern _ infixop _
operator <= alias LE pattern _ infixop _
operator >= alias GE pattern _ infixop _
operator # alias NE pattern _ infixop _

# need not be defined - is predefined this way in the Secondo parser:
# operator = alias EQ pattern _ infixop _  

operator not alias NOT pattern op ( _ )
operator and alias AND pattern  _ infixop _
operator or alias OR pattern  _ infixop _

operator starts alias STARTS pattern _ infixop _
operator contains alias CONTAINS pattern _ infixop _

operator isempty alias ISEMPTY pattern op ( _ )

operator intersection alias INTERSECTION pattern _ infixop _
operator minus alias SET_MINUS pattern _ infixop _

operator relcount alias RELCOUNT pattern _ op
operator relcount2 alias RELCOUNT2 pattern _ op

operator keywords alias KEYWORD pattern _ op



