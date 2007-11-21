

import java.io.*;
import java.util.StringTokenizer;
import java.util.Vector;

public class SeqPoly{

static class Point{

  public Point(double x, double y){
    this.x = x; 
    this.y = y;
  }

  public boolean equals(Object O){
    if(!(O instanceof Point)){
      return false;
    }
    Point P = (Point)O;
    return AlmostEqual(x,P.x) && AlmostEqual(y,P.y);
  }

  public String toString(){
     return "(" + x + " " + y +")";
  }
  private double x;
  private double y;
}


static int Number;
static boolean fresh;
static double lastX;
static double lastY;
static Vector region = new Vector();

static double EPSILON = 0.00001;


static void printFace(Vector v) {
   if(v.size() < 3){
      return;
   }
   System.out.println("((");  // start circle
   for(int i=0;i<v.size();i++){
      System.out.println(v.get(i));
   }
   System.out.println("))"); 

}

static void findAndPrintRegion(Vector allPoints){
   Vector tmp = new Vector(allPoints.size());

   if(allPoints.size()>0){
      // first step: remove equal points 
      Point lastP=null;
      for(int i=0;i<allPoints.size();i++){
         Point P = (Point) allPoints.get(i);
         if(i==0 || !P.equals(lastP)){
           tmp.add(P);
         }   
         lastP = P;
      }
    
 
      // find circles in tmp
      Vector tmp2 = new Vector(tmp.size());
      for(int i=0;i<tmp.size();i++){
          Object P = tmp.get(i);
          int index = tmp2.indexOf(P);
          if(index < 0 ){
            tmp2.add(P);
          } else { // Point already store in tmp2
             Vector circle = new Vector(tmp2.size());
             for(int j= index ; j< tmp2.size(); j++){
                 circle.add(tmp2.get(j));
             }
             printFace(circle);
             tmp2.setSize(index);
          }
      }
      printFace(tmp2);
   }
}



static boolean AlmostEqual(double a, double b){
  return Math.abs(a-b) <= EPSILON;
}


static void showUsage(){
  System.out.println(" java SeqPoly file [<Out>]");
  System.exit(0);

}

static void printHeader(){
   System.out.println("(OBJECT Polygon ()");
   System.out.println("   (rel (tuple( ( No int)(Reg region)))) ");
   System.out.println("  ( "); // open value 
}

static void printRest(){
   System.out.println("))"); // close value , close object
}

static void processLine(String line){
  if(line.equals("") ){
    if(!fresh){ // there is a region stored
      fresh = true;
      System.out.println("( " + Number + " ("); // open tuple , write Number, open region
      findAndPrintRegion(region);
      System.out.println("))"); // close region and tuple
      Number++;
    }

    region = new Vector();
  } else {
     fresh = false;
     StringTokenizer st = new StringTokenizer(line);
     double x = Double.parseDouble(st.nextToken()); 
     double y = Double.parseDouble(st.nextToken()); 
     region.add(new Point(x,y));
  }

} 

public static void main(String[] args){
  if(args.length<1){
    showUsage();
  }

  try {
      BufferedReader in = new BufferedReader(new FileReader(args[0]));
      printHeader();
      Number = 1;
      fresh = true;
      while(in.ready()){
        String line = in.readLine();
        processLine(line); 
      }
      printRest();
  } catch(Exception e ){
     e.printStackTrace();
  }

}


}
