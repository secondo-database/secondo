package viewer.hoese;

import project.*;


public class ProjectionManager {

   public static double getPrjX(double lambda, double phi) throws InvalidInputException{
      return P.getPrjX(lambda,phi);
   }

   public static double getPrjY(double lambda, double phi) throws InvalidInputException{
      return P.getPrjY(lambda,phi);
   }


   public static boolean showSettings(){
      return P.showSettings();
   }

   public static String getName(){
      return P.getName();
   }

   public static Projection getVoidProjection(){
      return VP;
   }

   public static Projection getActualProjection(){
      return P;
   }

   public static void setProjection(Projection Prj){
     if(Prj!=null)
       P = Prj;
   }


// private static  Projection P = new Mercator(); // later the equals-projection
// private static  Projection P = new Mollweide();
private static Projection P = new  VoidProjection();

private static Projection VP = new VoidProjection();

}
