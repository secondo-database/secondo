import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;
import java.io.*;
import java.util.*;

public class Lines implements Serializable{
    //this class implements the Lines value of the ROSE algebra

    //members
    public SegMultiSet segset; //the set of segments
    
    //constructors
    public Lines() {
	//System.out.println("--> constructed an empty LINES object");
	segset = new SegMultiSet(new SegmentComparator());
    }

    public Lines(SegMultiSet sl) {
	segset = sl;//SegMultiSet.convert(sl.copy());
    }

    //methods
    public void add(double x1, double y1, double x2, double y2) {
	//constructs a new Segment and adds it to this
	this.segset.add(new Segment(x1,y1,x2,y2));
    }//end method add

    public void add(Rational x1, Rational y1, Rational x2, Rational y2) {
	//constructs a new segment and adds it to this
	this.segset.add(new Segment(x1,y1,x2,y2));
    }//end method add

    public void add(int x1, int y1, int x2, int y2) {
	//constructs a new segment
	this.segset.add(new Segment(x1,y1,x2,y2));
    }//end method add

    public void add(Segment s) {
	//adds s to segset
	this.segset.add(s);
    }//end method add
	
    public static Lines readFrom(byte[] buffer){
	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Lines res = (Lines) ois.readObject();
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
    
    /*
    public int compare (Lines lIn) {
	//returns 0 if this == pin
	//as long as elements in sorted lists from the beginning to the
	//end are equal, traverse through the lists.
	//when the first elements are found which are not equal, then
	//return -1 if this has the smaller element
	//return +1 if lIn has the smaller element
	//if one list has less elements than the other and the first elements
	//are equal, then
	//return -1 if this is shorter than lIn
	//return +1 if lIn is shorter than this

	//first, align an sort both SegSets
	SegList thiscop = (SegList)this.seglist.clone();
	SegList lincop = (SegList)lIn.seglist.clone();
	
	ListIterator lit1 = thiscop.listIterator(0);
	ListIterator lit2 = lincop.listIterator(0);
	
	while (lit1.hasNext()) ((Segment)lit1.next()).align();
	while (lit2.hasNext()) ((Segment)lit2.next()).align();

	SetOps.quicksortX(thiscop);
	SetOps.quicksortX(lincop);

	lit1 = thiscop.listIterator(0);
	lit2 = thiscop.listIterator(0);

	Segment actS1;
	Segment actS2;
	byte res;
	while (lit1.hasNext() && lit2.hasNext()) {
	    actS1 = (Segment)lit1.next();
	    actS2 = (Segment)lit2.next();
	    res = actS1.compare(actS2);
	    if (!(res == 0)) return (int)res;
	}//while
	if (!lit1.hasNext() && !lit2.hasNext()) return 0;
	if (!lit1.hasNext()) return -1;
	else return 1;
    }//end method compare
    */
    /*
    public Lines copy () {
	return new Lines(this.seglist);
    }//end method copy
    */
	
}//end class Lines
    
