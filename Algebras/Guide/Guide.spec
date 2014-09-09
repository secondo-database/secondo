
# simple prefix operator having one argument
operator perimeter alias PERIMETER pattern op(_)

# infix operator
operator distN alias DISTN pattern _ infixop _

# postfix operator having a parameter in square brackets
operator countNumber alias COUNTNUMBER pattern _ op[_]

#simple prefix operator
operator getChars alias GETCHARS pattern op(_)

# operator having a function with implicit arguments
operator replaceElem alias REPLACEELEM pattern _ op[ fun ] 
         implicit parameter streamelem type STREAMELEM

# postfix operator with an additional argument
operator attrIndex alias ATTRINDEX pattern _ op [_]



