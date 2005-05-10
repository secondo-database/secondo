package twodsack.util.graph;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;

class Edge implements ComparableMSE {
    
    public static final double EPSILON = 0.01; //used in method rat

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

    public Vertex theOtherOne(Vertex inVertex) {
	//returns that one of both vertices which is
	//not equal to inVertex
	if (this.first.equal(inVertex)) return this.second;
	else if (this.second.equal(inVertex)) return this.first;
	else {
	    throw new NoEqualVertexFoundException();
	}//else
    }//end method theOtherOne


    public int compare(ComparableMSE inE) {
	//returns -1,0,1 w.r.t. order of halfsegments(!)
	//This special order is defined in the ROSE
	//implementation paper
	
	Edge inEdge;

	if (!(inE instanceof Edge)) throw new WrongTypeException("Expected "+this.getClass()+", found "+inE.getClass());
	else inEdge = (Edge)inE;

	//The comparison below is only for edges that consist
	//of two points. A more generic implementation is given 
	//first.

	//generic implementation for edge = (element,element)
	if (!(inEdge.first.value instanceof Point)) {
	    Element el1 = (Element)inEdge.first.value;
	    Element el2 = (Element)inEdge.second.value;
	    return el1.compare(el2);
	}//if

	//here comes the implementation for edge = (point,point)

	//first, find the vertex connecting this and inEdge
	Vertex myDV; //dominating vertex of this
	Vertex myOV; //other vertex of this
	Vertex inDV; //dominating vertex of inEdge
	Vertex inOV; //other vertex of inEdge
	if (this.first.equal(inEdge.first)) {
	    myDV = this.first;
	    myOV = this.second;
	    inDV = inEdge.first;
	    inOV = inEdge.second;
	}//if
	else if (this.first.equal(inEdge.second)) {
	    myDV = this.first;
	    myOV = this.second;
	    inDV = inEdge.second;
	    inOV = inEdge.first;
	}//else if
	else if (this.second.equal(inEdge.first)) {
	    myDV = this.second;
	    myOV = this.first;
	    inDV = inEdge.first;
	    inOV = inEdge.second;
	}//else if
	else {
	    myDV = this.second;
	    myOV = this.first;
	    inDV = inEdge.second;
	    inOV = inEdge.first;
	}//else
	
	int compDPs = ((Element)myDV.value).compare((Element)inDV.value);
	if (compDPs != 0) return compDPs;

	if ((((Element)myDV.value).compare((Element)myOV.value) == 1) &&
	    (((Element)inDV.value).compare((Element)inOV.value) == -1)) return -1;
	
	    if ((((Element)myDV.value).compare((Element)myOV.value) == -1) &&
		(((Element)inDV.value).compare((Element)inOV.value) == 1)) return 1;

	if (rot(inEdge)) return -1;

	if (inEdge.rot(this)) return 1;

	//System.out.println("gone through all...");

	return 0;
    }//end method compare

    private boolean rot (Edge inEdge) {
	//supportive method for compare
	//returns true if inEdge is rot(atable)
	//on this in a degree A with 0 < A <= 180
	//CAUTION: This only works for Edges/Vertices
	//with Points as values! Hence an edge represents
	//a segment.
	//System.out.println("entering Edge.rot...");

	if (!(inEdge.first.value instanceof Point)) {
	    System.out.println("Error in Edge.rot: this method only works with Point(s) --> found "+inEdge.first.value.getClass()+".");
	    System.out.println("execution terminated.");
	    System.exit(0);
	}//if
	
	double x1,x2,y1,y2;
	Vertex myDV; //dominating vertex of both edges(segments)
	Vertex myOV; //other Vertex
	if (this.first.equal(inEdge.first)) {
	    myDV = this.first;
	    myOV = this.second;
	}//if
	else {
	    myDV = this.second;
	    myOV = this.first;
	}//else

	//compute vector representation for both edges(segments)
	if (((Point)myDV.value).compare((Point)myOV.value) == -1) {
	    x1 = ((Point)myOV.value).x.minus(((Point)myDV.value).x).getDouble();
	    y1 = ((Point)myOV.value).y.minus(((Point)myDV.value).y).getDouble();
	}//if
	else {
	    x1 = ((Point)myDV.value).x.minus(((Point)myOV.value).x).getDouble();
	    y1 = ((Point)myDV.value).y.minus(((Point)myOV.value).y).getDouble();
	}//else

	Vertex inDV,inOV;
	if (myDV.equal(inEdge.first)) {
	    inDV = inEdge.first;
	    inOV = inEdge.second;
	}//if
	else {
	    inDV = inEdge.second;
	    inOV = inEdge.first;
	}//else

	if (((Point)inDV.value).compare((Point)inOV.value) == -1) {
	    x2 = ((Point)inOV.value).x.minus(((Point)inDV.value).x).getDouble();
	    y2 = ((Point)inOV.value).y.minus(((Point)inDV.value).y).getDouble();
	}//if
	else {
	    x2 = ((Point)inDV.value).x.minus(((Point)inOV.value).x).getDouble();
	    y2 = ((Point)inDV.value).y.minus(((Point)inOV.value).y).getDouble();
	}//else

	//compute the angle between the vectors
	double param = (x1*x2+y1*y2) /
	    (Math.sqrt(x1*x1+y1*y1) *
	     Math.sqrt(x2*x2+y2*y2));
	if (param < -1) param = -1;
	if (param > 1) param = 1;
	double angle = Math.abs(Math.acos((x1*x2+y1*y2) /
				(Math.sqrt(x1*x1+y1*y1) *
				 Math.sqrt(x2*x2+y2*y2))));

	//System.out.println("first angle in rot: "+Math.toDegrees(angle));

	if (angle <= EPSILON) return false; //overlapping segments!

	//compute a rotated vector (rotation by angle)
	double x3 = x2*Math.cos(angle) + y2*Math.sin(angle);
	double y3 = -x2*Math.sin(angle) + y2*Math.cos(angle);
		    
	//compute the angle between the rotated vector and inEdge
	param = (x3*x1+y3*y1) /
	    (Math.sqrt(x3*x3+y3*y3) *
	     Math.sqrt(x1*x1+y1*y1));
	if (param < -1) param = -1;
	if (param > 1) param = 1;
	angle = Math.abs(Math.acos(param));

	//System.out.println("second angle in rot: "+Math.toDegrees(angle));

	if (angle <= EPSILON) return true;
	else return false;
       
    }//end method rot

}//end class Edge
