operator distribute alias DISTRIBUTE pattern  _ op [ _ ]
operator summarize alias SUMMARIZE pattern _ op
operator loop alias LOOP pattern _ op [ fun ] implicit parameter element type ELEMENT
operator loop2 alias LOOP2 pattern _ _ op [ fun ] implicit parameters first, second types ELEMENT, ELEMENT2
operator sortarray alias SORTARRAY pattern _ op [ fun ] implicit parameter element type ELEMENT
operator get alias GET pattern _ op [ _ ]
operator put alias PUT pattern _ _ op [ _ ]
operator tie alias TIE pattern _ op [ fun ] implicit parameters first, second types ELEMENT, ELEMENT
operator cumulate alias CUMULATE pattern _ op [ fun ] implicit parameters first, second types ELEMENT, ELEMENT

