import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;
import java.lang.reflect.*;
import java.io.*;
import java.util.*;

public class Points implements Serializable{
    //this class implements the Points value of the ROSE algebra

    //members
    public PointMultiSet pointset; //the set of points
    
    //constructors
    public Points() {
	//System.out.println("--> constructed an empty POINTS object");
	pointset = new PointMultiSet(new PointComparator());
    }

    public Points(PointMultiSet pl) {
	//System.out.println("--> constructed a POINTS object from a pointset");
	//pointset = PointMultiSet.convert(pl.copy());
	pointset = pl;
    }

    //methods
    public int size() {
	return this.pointset.size();
    }//end method size


    public void add(Point p) {
	//adds p to this
	this.pointset.add(p);
    }//end method add

    public void add(Rational x, Rational y) {
	//constructs a new Point object and adds it to this
	this.pointset.add(new Point(x,y));
    }//end method add

    public void add(int x, int y) {
	//constructs a new Point object and adds it to this
	this.pointset.add(new Point(x,y));
    }//end method add

    public void add(double x, double y) {
	//constructs a new Point object and adds it to this
	this.pointset.add(new Point(x,y));
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


    /** this method serializes an object */
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
	    System.out.println("Error in Points.writeToByteArray."+e); 
	    e.printStackTrace();
	    return null;
	}
    }//end method writeToByteArray

    /* DEACTIVATED W.R.T. MULTISETS
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

	//first, sort both pointsets
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
    */

    public Points copy () {
	return new Points(PointMultiSet.convert(this.pointset.copy()));
    }//end method copy

    
    public void print() {
	this.pointset.print();
    }


}//end class Points
    
