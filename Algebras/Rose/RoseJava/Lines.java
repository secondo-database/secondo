import java.io.*;

public class Lines implements Serializable{
    //this class implements the Lines value of the ROSE algebra

    //members
    public SegList seglist; //the list of segments
    
    //constructors
    public Lines() {
	seglist = new SegList();
    }

    public Lines(SegList sl) {
	seglist = SegList.convert(sl.copy());
    }

    //methods

}//end class Lines
    
