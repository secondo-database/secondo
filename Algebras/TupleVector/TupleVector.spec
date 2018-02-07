


operator feedv alias FEEDV pattern _ op[_]

operator count alias COUNT pattern _ op
operator countv alias COUNTV pattern _ op

operator projectv alias PROJECTV pattern _ op [_,_]
operator projectnp alias PROJECTNP pattern _ op [_,_]

operator consume alias CONSUME pattern _ op

operator filterv alias FILTERV pattern _ op[fun,_] implicit parameter elem type TVS2T
operator filternp alias FILTERNP pattern _ op[fun] implicit parameter elem type STREAMELEM

operator tvs2ts alias TVS2TS pattern _ op 
operator ts2tvs alias TS2TVS pattern _ op[_]

operator extendv alias EXTENDV pattern _ op[funlist] implicit parameter elem type TVS2T
operator projectextendv alias PROJECTEXTENDV pattern _ op[list;funlist] implicit parameter elem type TVS2T



operator feednp alias FEEDNP pattern _ op
operator countnp alias COUNTNP pattern _ op
operator extendnp alias EXTENDNP pattern _ op[funlist] implicit parameter elem type STREAMELEM
operator projectextendnp alias PROJECTEXTENDNP pattern _ op[list;funlist] implicit parameter elem type STREAMELEM





