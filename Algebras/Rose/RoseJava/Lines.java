import java.io.*;

public class Lines implements Serializable{
    //this class implements the Lines value of the ROSE algebra

    //members
    public SegList seglist; //the list of segments
    
    //constructors
    public Lines() {
	System.out.println("--> constructed an empty LINES object");
	seglist = new SegList();
    }

    public Lines(SegList sl) {
	seglist = SegList.convert(sl.copy());
    }

    //methods
    public void add(double x1, double y1, double x2, double y2) {
	//constructs a new Segment and adds it to this
	this.seglist.add(new Segment(x1,y1,x2,y2));
    }//end method add

    public void add(Rational x1, Rational y1, Rational x2, Rational y2) {
	//constructs a new segment and adds it to this
	this.seglist.add(new Segment(x1,y1,x2,y2));
    }//end method add

    public void add(int x1, int y1, int x2, int y2) {
	//constructs a new segment
	this.seglist.add(new Segment(x1,y1,x2,y2));
    }//end method add

    public void add(Segment s) {
	//adds s to seglist
	this.seglist.add(s);
    }//end method add
	
	
}//end class Lines
    
