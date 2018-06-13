
package extern.gpxreader;

import sj.lang.ListExpr;
import extern.SecondoImporter;

public class GpxReader implements SecondoImporter{

  public ListExpr getList(String fileName){
     return null;
  }

  public String getErrorString(){
     return "gpx import not implemented yet";
  }

  public String getFileDescription(){
    return "a single gpx-file or a directory containing gpx-files";
  }  

}

