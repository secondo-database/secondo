class ConnectedComponentsPair {

    //members
    ElemListList compVertices;
    PairListList compEdges;

    //constructors
    ConnectedComponentsPair() {
	this.compVertices = new ElemListList();
	this.compEdges = new PairListList();
    }

    ConnectedComponentsPair(ElemListList v, PairListList e) {
	this.compVertices = v;
	this.compEdges = e;
    }

    //methods



}//end class ConnectedComponentsPair
