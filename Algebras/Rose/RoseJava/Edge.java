class Edge {
    
    //members
    public Vertex first;
    public Vertex second;

    //constructors
    Edge() {
	first = null;
	second = null;
    }

    Edge(Vertex v1, Vertex v2) {
	first = v1.copy();
	second = v2.copy();
    }

    //methods
    public void print() {
	System.out.println("Edge:");
	this.first.print();
	this.second.print();
    }//end method print

}//end class Edge
