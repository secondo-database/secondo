import sj.lang.ListExpr;
import viewer.hoese.*;
import java.io.*;

/**
  * This class creates a moving Region like described in the article
  * External Representation of Spatial and Spatio-Temporal Values
**/

public class RegTransLinear{


private static ListExpr processPoint(ListExpr PL, ATransform AT){
   Point P = new Point();
   if(!P.readFrom(PL)){
       System.err.println("error in reading point");
       System.exit(65); // data format error
   }
   AT.transformPoint(P);
   if(firstRun) points++;
   return ListExpr.twoElemList(ListExpr.realAtom((float)P.getX()),ListExpr.realAtom((float)P.getY()));
}

/**
  * transforms a single Cycle of a region
**/
private static ListExpr processCycle(ListExpr Cycle,ATransform AT){
   if(Cycle.isEmpty())
      return  new ListExpr();

   ListExpr Res = ListExpr.oneElemList(processPoint(Cycle.first(),AT));
   ListExpr Last = Res;
   Cycle = Cycle.rest();
   while(!Cycle.isEmpty()){
      Last = ListExpr.append(Last,processPoint(Cycle.first(),AT));
      Cycle = Cycle.rest();
   }
   return Res;
}

/*
 * transforms a single face of a region
*/
private static ListExpr processFace(ListExpr Face, ATransform AT){
   if(Face.isEmpty())
      return new ListExpr();

   ListExpr Res = ListExpr.oneElemList(processCycle(Face.first(),AT));
   Face = Face.rest();
   ListExpr Last = Res;

   while(!Face.isEmpty()){
      Last = ListExpr.append(Last,processCycle(Face.first(),AT));
      Face = Face.rest();
   }
   return Res;
}


/**
 * Transforms Region with the given transformation.
 * The Result will be the transformed region in nested list format.
**/
private static ListExpr processValueList(ListExpr Region, ATransform AT){
  if(Region.isEmpty())
     return new ListExpr();

  ListExpr Res = ListExpr.oneElemList(processFace(Region.first(),AT));
  ListExpr Last = Res;
  Region = Region.rest();
  while(!Region.isEmpty()){
     Last = ListExpr.append(Last,processFace(Region.first(),AT));
     Region = Region.rest();
  }
  return Res;
}



private static void writePointMap(ListExpr Start, ListExpr End){
   Double X1 = LEUtils.readNumeric(Start.first());
   Double Y1 = LEUtils.readNumeric(Start.second());
   Double X2 = LEUtils.readNumeric(End.first());
   Double Y2 = LEUtils.readNumeric(End.second());
   if(X1==null || X2==null || Y1==null || Y2 == null){
      System.err.println("Error in writing PointMap");
      System.err.println(" The Lists are :");
      System.err.println(" Start ="+Start.writeListExprToString());
      System.err.println(" End ="+End.writeListExprToString());
      System.exit(65); // data format error
   }
   out.println("("+X1.doubleValue()+" "+Y1.doubleValue()+" "+
                          X2.doubleValue()+" "+Y2.doubleValue()+")");

}

/** write a single cyclemap to the standard output **/
private static void writeCycleMap(ListExpr Start, ListExpr End){
  out.println("("); // open map
  while(!Start.isEmpty()){
      writePointMap(Start.first(),End.first());
      Start = Start.rest();
      End = End.rest();
  }
  out.println(")"); // close map
}

/** write a single facemap to the standard output **/
private static void writeFaceMap(ListExpr Start, ListExpr End){
  out.println("("); // open map
  while(!Start.isEmpty()){
      writeCycleMap(Start.first(),End.first());
      Start = Start.rest();
      End = End.rest();
  }
  out.println(")"); // close map
}

/** write the Region map to the standardoutput */
private static void writeRegMap(ListExpr Start, ListExpr End){
  out.println("("); // open map
  while(!Start.isEmpty()){
      writeFaceMap(Start.first(),End.first());
      Start = Start.rest();
      End = End.rest();
  }
  out.println(")"); // close map
}

/**This function writes a single unit computed from the arguments to the
  * standard output
  **/
private static void writeUnit(double startTime, double moveTime,ListExpr Start, ListExpr End){
   out.println("(");  // open unit
     out.print("  ("); // open interval
     out.print(DateTime.getListString(startTime,false));
     out.print(" "+DateTime.getListString(startTime+moveTime,false));
     out.print(" TRUE FALSE ");
     out.println(" )"); // close interval
     writeRegMap(Start,End);  // write the map
     out.println(")"); // close unit
}



public static void main(String[] args){
  if(args.length<3){
     System.err.println ("missing argument");
     System.err.println("usage:java RegTransformer RegionFile TransformFile outFile [init]");
     System.exit(64); // command line usage error
  }
  ListExpr RegList = new ListExpr();
  if(RegList.readFromFile(args[0])!=0){
    System.err.println(" error in reading RegionFile");
    System.exit(65);
  }
  ListExpr TransformList = new ListExpr();
  if(TransformList.readFromFile(args[1])!=0){
     System.err.println("error in reading transformation file");
     System.exit(65);
  }

  if(RegList.listLength()!=2){
     System.err.println("RegionFile contains not a Region");
     System.exit(65);
  }

  if(RegList.first().atomType()!=RegList.SYMBOL_ATOM || !RegList.first().symbolValue().equals("region")){
     System.err.println("RegionFile contains not a Region");
     System.exit(65);
  }
  try{
    out = new PrintStream(new FileOutputStream(args[2]));
  }catch(Exception e){
    System.err.println("error in opening output file"); 
    System.exit(73); // can't create user output file
  }
  init = false;
  if(args.length>3 && args[3].equals("init"))
    init = true;

  ATransform AT = new ATransform();
  ListExpr RValue = RegList.second();
  ListExpr NextValue;
  firstRun=true; // only needed to count the number of
                 // contained points
  double startTime = 0.0;
  double moveTime = 0.25; // this means 6 hours
  out.println("( movingregion "); // oopen object and write type
  out.println("("); // open value
  firstUnit =true;
  while(!TransformList.isEmpty()){
     if(!AT.readFrom(TransformList.first())){
       System.err.println("error in TransformationList");
       System.exit(65);
     }
     TransformList = TransformList.rest();
     NextValue = processValueList(RValue,AT);
     firstRun=false;
     if(!init || !firstUnit){ 
        writeUnit(startTime,moveTime,RValue,NextValue);
     } else{
        firstUnit=false;
     }
     startTime += moveTime;
     RValue = NextValue;
  }

    out.println("))"); // close value and object
    System.err.println("processed "+points+" points");
    try{
       out.close();
    }catch(Exception e){
       System.err.println("Warning: Error in Closing output file");
    }
}
  static int points;
  static boolean firstRun;
  static PrintStream out;
  static boolean init;
  static boolean firstUnit;
}
