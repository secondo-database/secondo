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

}//end class Vertex
    
