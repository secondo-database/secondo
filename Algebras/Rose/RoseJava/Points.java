import java.lang.reflect.*;
import java.io.*;

public class Points implements Serializable{
    //this class implements the Points value of the ROSE algebra

    //members
    public PointList pointlist; //the list of points
    
    //constructors
    public Points() {
	pointlist = new PointList();
    }

    public Points(PointList pl) {
	pointlist = PointList.convert(pl.copy());
    }

    //methods

}//end class Points
    
