/*
Secondo Prolog Interface

*/

class SecondoPrologInterface
{

  public:
	
    void startPrologEnginge();

    ListExpr callPrologQueryTransform(ListExpr expr, NestedList* nl);

    term_t ListExprToTermForInterface(ListExpr expr, NestedList* nl);

    ListExpr TermToListExpr(term_t t, NestedList* nl, bool& error);

    ListExpr AtomToListExpr(NestedList* nl, char* str, bool& error);
};
