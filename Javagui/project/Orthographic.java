package project;



public class Orthographic implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
      double l1 = (lambda-Lambda_0)*PI/180;
      double p1 = phi*PI/180;
      double x= Math.cos(p1)*Math.sin(l1);
      return x*180/PI;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
       double l1 = (lambda-Lambda_0)*PI/180;
       double p1 = phi*PI/180;
       double y = Math.cos(Phi_1)*Math.sin(p1)-Math.sin(Phi_1)*Math.cos(p1)*Math.cos(l1);
       return y*180/PI;
   }

   public boolean showSettings(){
     System.out.println("Orthographic.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Orthographic";
   }


   private double Lambda_0 = 0;
   private double Phi_1 = 0*PI/180;
   private double secure_distance = 10;

}
