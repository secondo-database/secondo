package extern.shapereader;

import extern.numericreader.*;

public class ShapeType{

// constants defined by shapefile description
public static final int NULL_SHAPE = 0;
public static final int POINT = 1;
public static final int POLYLINE = 3;
public static final int POLYGON = 5;
public static final int MULTIPOINT = 8;
public static final int POINT_Z = 11;
public static final int POLYLINE_Z = 13;
public static final int POLYGON_Z = 15;
public static final int MULTIPOINT_Z = 18;
public static final int POINT_M = 21;
public static final int POLYLINE_M = 23;
public static final int POLYGON_M = 25;
public static final int MULTIPOINT_M = 28;
public static final int MULTIPATCH = 31;

public static boolean isKnow(int type){
  return (type==NULL_SHAPE)   | (type==POINT) |
         (type==POLYLINE)     | (type==POLYGON) |
         (type==MULTIPOINT)   | (type==POINT_Z) |
         (type==POLYLINE_Z)   | (type==POLYGON_Z) |
         (type==MULTIPOINT_Z) | (type==POINT_M) |
         (type==POLYLINE_M)   | (type==POLYGON_M) |
         (type==MULTIPOINT_M) | (type==MULTIPATCH);
}


public static String getName(int type){
   switch(type){
      case NULL_SHAPE     : return "Null_Shape";
      case POINT          : return "Point";
      case POLYLINE       : return "Polyline";
      case POLYGON        : return "Polygon";
      case MULTIPOINT     : return "Multipoint";
      case POINT_Z        : return "Point Z";
      case POLYLINE_Z     : return "Polyline Z";
      case POLYGON_Z      : return "Polygon Z";
      case MULTIPOINT_Z   : return "Multipoint Z";
      case POINT_M        : return "Point M";
      case POLYLINE_M     : return "Polyline M";
      case POLYGON_M      : return "Polgon M";
      case MULTIPOINT_M   : return "Multipoint M";
      case MULTIPATCH     : return "Multipatch";
      default             : return "UNKNOW";
   }
}

public static String getSecondoName(int type){
   switch(type){
      case NULL_SHAPE     : return "null";
      case POINT          : return "point";
      case POLYLINE       : return "line";
      case POLYGON        : return "region";
      case MULTIPOINT     : return "points";
      case POINT_Z        : return "point3d";
      case POLYLINE_Z     : return "line3d";
      case POLYGON_Z      : return "area3d";
      case MULTIPOINT_Z   : return "points3d";
      case POINT_M        : return "pointm";
      case POLYLINE_M     : return "linem";
      case POLYGON_M      : return "regionm";
      case MULTIPOINT_M   : return "pointsm";
      case MULTIPATCH     : return "multipatch";
      default             : return "UNKNOW";
   }
}


 public int getType(){
    return type;
 }

 public boolean readFrom(char[] Buffer){
   if(Buffer.length!=4)
      return false;
   type = NumericReader.getIntLittle(Buffer,0);
   return true;
 }

 private int type=0;

}
