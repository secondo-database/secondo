import java.lang.reflect.*;
import java.io.*;

public class Points implements Serializable{
    //this class implements the Points value of the ROSE algebra

    //members
    public PointList pointlist; //the list of points
    
    //constructors
    public Points() {
	System.out.println("--> constructed an empty POINTS object");
	pointlist = new PointList();
    }

    public Points(PointList pl) {
	System.out.println("--> constructed a POINTS object from a pointlist");
	pointlist = PointList.convert(pl.copy());
    }

    //methods
    public void add(Point p) {
	//adds p to this
	this.pointlist.add(p);
    }//end method add

    public void add(Rational x, Rational y) {
	//constructs a new Point object and adds it to this
	this.pointlist.add(new Point(x,y));
    }//end method add

    public void add(int x, int y) {
	//constructs a new Point object and adds it to this
	this.pointlist.add(new Point(x,y));
    }//end method add

    public void add(double x, double y) {
	//constructs a new Point object and adds it to this
	this.pointlist.add(new Point(x,y));
    }//end method add
	
}//end class Points
    
