# Begin ArrayAlgebra.spec

operator get alias GET pattern op ( _, _ )

operator put alias PUT pattern op ( _, _, _ )

operator sortarray alias SORTARRAY pattern _ op [ fun ]
         implicit parameter element type ELEMENT

operator tie alias TIE pattern _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT

operator cumulate alias CUMULATE pattern _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT

operator distribute alias DISTRIBUTE pattern _ op [ _ ]

operator summarize alias SUMMARIZE pattern _ op

operator loop alias LOOP pattern _ op [ fun ] 
         implicit parameter element type ELEMENT

operator loopa alias LOOPA pattern _ _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT2

operator loopb alias LOOPB pattern _ _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT2

operator loopswitch alias LOOPSWITCH pattern _ op [ funlist ] 
         implicit parameter element type ELEMENT

operator loopswitcha alias LOOPSWITCHA pattern _ _ op [ funlist ]

operator loopswitchb alias LOOPSWITCHB pattern _ _ op [ funlist ]

operator loopselect alias LOOPSELECT pattern _ op [ funlist; _, _ ] 
         implicit parameter element type ELEMENT

operator loopselecta alias LOOPSELECTA pattern _ _ op [ funlist; _, _ ]

operator loopselectb alias LOOPSELECTB pattern _ _ op [ funlist; _, _ ]

operator partjoin alias PARTJOIN pattern _ _ op [ fun ] 
         implicit parameters first, second types ELEMENT, ELEMENT2

operator partjoinswitch alias PARTJOINSWITCH pattern _ _ op [ funlist ]

operator partjoinselect alias PARTJOINSELECT pattern _ _ op [ funlist; _, _ ]

# End ArrayAlgebra.spec
