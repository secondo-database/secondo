import java.lang.reflect.*;
import java.io.*;
import java.util.*;

public class Points implements Serializable{
    //this class implements the Points value of the ROSE algebra

    //members
    public PointList pointlist; //the list of points
    
    //constructors
    public Points() {
	//System.out.println("--> constructed an empty POINTS object");
	pointlist = new PointList();
    }

    public Points(PointList pl) {
	//System.out.println("--> constructed a POINTS object from a pointlist");
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


    public static Points readFrom(byte[] buffer){
	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Points res = (Points) ois.readObject();
	    ois.close();
	    return res;
	} catch(Exception e){
	    return null;
	}
    }//end method readFrom


    /** this method serialized an object */
    public  byte[] writeToByteArray(){
	
	try{
	    ByteArrayOutputStream byteout = new ByteArrayOutputStream();
	    ObjectOutputStream objectout = new ObjectOutputStream(byteout);
	    objectout.writeObject(this);
	    objectout.flush();
	    byte[] res = byteout.toByteArray();
	    objectout.close();
	    return  res;
	} catch(Exception e){
	    return null;
	}
    }//end method writeToByteArray


    public int compare (Points pIn) {
	//returns 0 if this == pin
	//as long as elements in sorted lists from the beginning to the
	//end are equal, traverse through the lists.
	//when the first elements are found which are not equal, then
	//return -1 if this has the smaller element
	//return +1 if pin has the smaller element
	//if one list has less elements than the other and the first elements
	//are equal, then 
	//return -1 if this is shorter than pIn
	//return +1 if pIn is shorter than this

	//first, sort both pointlists
	PointList thiscop = (PointList)this.pointlist.clone();
	PointList pincop = (PointList)pIn.pointlist.clone();
	
	SetOps.quicksortX(thiscop);
	SetOps.quicksortX(pincop);

	ListIterator lit1 = thiscop.listIterator(0);
	ListIterator lit2 = pincop.listIterator(0);

	Point actP1;
	Point actP2;
	byte res;
	while (lit1.hasNext() && lit2.hasNext()) {
	    actP1 = (Point)lit1.next();
	    actP2 = (Point)lit2.next();
	    res = actP1.compare(actP2);
	    if (!(res == 0)) return (int)res;
	}//while
	if (!lit1.hasNext() && !lit2.hasNext()) return 0;
	if (!lit1.hasNext()) return -1;
	else return 1;
    }//end method compare

    public Points copy () {
	return new Points(this.pointlist);
    }//end method copy

	
}//end class Points
    
