class Vertex {
    //provides a vertex for class Graph

    //members
    public Element value;
    public int number;

    //constructors
    public Vertex(Element val, int num) {
	value = val;
	number = num;
    }

    //methods
    public void print() {
	System.out.print(number+": ");
	value.print();
    }//end method print

    public Vertex copy() {
	return new Vertex(this.value,this.number);
    }//end method copy

    public boolean equal(Vertex inVer) {
	if (inVer.value.equal(this.value)) return true;
	else return false;
    }//end method equal

}//end class Vertex
    
