package project;



public class Sinusoidal implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
      double phi_1 = phi*PI/180;
      double L_1 = (lambda-Lambda_0)*PI/180;
      double rr = L_1*Math.cos(phi_1);
      return rr*180/PI;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     return phi;
   }

   public boolean showSettings(){
     System.out.println("Sinusoisal.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Sinusoidal";
   }


   private double Lambda_0 = 0;
   private double secure_distance = 1;

}
