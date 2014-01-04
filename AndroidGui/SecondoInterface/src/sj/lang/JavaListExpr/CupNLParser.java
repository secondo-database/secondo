package sj.lang.JavaListExpr;

import sj.lang.ListExpr;

/** Cup generated class to encapsulate user supplied action code.*/
class CUP$NLParser$actions {
  private final NLParser parser;

  /** Constructor */
  CUP$NLParser$actions(NLParser parser) {
    this.parser = parser;
  }

  /** Method with the actual generated action code. */
  public final java_cup10.runtime.Symbol CUP$NLParser$do_action(
    int                        CUP$NLParser$act_num,
    java_cup10.runtime.lr_parser CUP$NLParser$parser,
    java.util.Stack            CUP$NLParser$stack,
    int                        CUP$NLParser$top)
    throws java.lang.Exception
    {
      /* Symbol object for return from actions */
      java_cup10.runtime.Symbol CUP$NLParser$result;

      /* select the action based on the action number */
      switch (CUP$NLParser$act_num)
        {
          /*. . . . . . . . . . . . . . . . . . . .*/
          case 10: // list_expr_seq ::= 
            {
              Stack RESULT = null;
		
		    RESULT = new Stack();
                  
              CUP$NLParser$result = new java_cup10.runtime.Symbol(3/*list_expr_seq*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 9: // list_expr_seq ::= list_expr_seq list_expr 
            {
              Stack RESULT = null;
		int listSeqleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).left;
		int listSeqright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).right;
		Stack listSeq = (Stack)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).value;
		int listExprleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int listExprright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr listExpr = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                    // The value of a ~list_expr_seq~ is stored as a stack,
                    // because for using the ~cons()~ method of the ~ListExpr~
                    // class we need to use the elements in reverse order to
                    // the one with which we get them. So we store all them
                    // and when we find apply the rule 1 we build the
                    // equivalent ListExpr value.
                    listSeq.push(listExpr);
                    RESULT = listSeq;
                  
              CUP$NLParser$result = new java_cup10.runtime.Symbol(3/*list_expr_seq*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 8: // list_expr ::= TT_TEXT_ATOM 
            {
              ListExpr RESULT = null;
		int atomleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int atomright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr atom = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                RESULT = atom;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 7: // list_expr ::= TT_SYMBOL_ATOM 
            {
              ListExpr RESULT = null;
		int atomleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int atomright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr atom = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                RESULT = atom;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 6: // list_expr ::= TT_STRING_ATOM 
            {
              ListExpr RESULT = null;
		int atomleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int atomright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr atom = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                RESULT = atom;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 5: // list_expr ::= TT_REAL_ATOM 
            {
              ListExpr RESULT = null;
		int atomleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int atomright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr atom = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                RESULT = atom;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 4: // list_expr ::= TT_BOOL_ATOM 
            {
              ListExpr RESULT = null;
		int atomleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int atomright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr atom = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                RESULT = atom;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 3: // list_expr ::= TT_INT_ATOM 
            {
              ListExpr RESULT = null;
		int atomleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int atomright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr atom = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		
                RESULT = atom;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 2: // list_expr ::= TT_OPEN_PAR list_expr_seq TT_CLOSE_PAR 
            {
              ListExpr RESULT = null;
		int listSeqleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).left;
		int listSeqright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).right;
		Stack listSeq = (Stack)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).value;
		
                 // It takes the Stack in ~listSeq~ and builds the
                 // equivalent ListExpr.
                 ListExpr auxList = ListExpr.theEmptyList();
		 while(!listSeq.isEmpty())
                    auxList= ListExpr.cons((ListExpr)listSeq.pop(), auxList);
                 RESULT = auxList;
              
              CUP$NLParser$result = new java_cup10.runtime.Symbol(1/*list_expr*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-2)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 1: // ok ::= list_expr 
            {
              ListExpr RESULT = null;
		int list1left = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left;
		int list1right = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right;
		ListExpr list1 = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).value;
		 
            RESULT = list1;
         
              CUP$NLParser$result = new java_cup10.runtime.Symbol(2/*ok*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          return CUP$NLParser$result;

          /*. . . . . . . . . . . . . . . . . . . .*/
          case 0: // $START ::= ok EOF 
            {
              Object RESULT = null;
		int start_valleft = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).left;
		int start_valright = ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).right;
		ListExpr start_val = (ListExpr)((java_cup10.runtime.Symbol) CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).value;
		RESULT = start_val;
              CUP$NLParser$result = new java_cup10.runtime.Symbol(0/*$START*/, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-1)).left, ((java_cup10.runtime.Symbol)CUP$NLParser$stack.elementAt(CUP$NLParser$top-0)).right, RESULT);
            }
          /* ACCEPT */
          CUP$NLParser$parser.done_parsing();
          return CUP$NLParser$result;

          /* . . . . . .*/
          default:
            throw new Exception(
               "Invalid action number found in internal parse table");

        }
    }
}