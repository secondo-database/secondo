package project;



public class PolyConic implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
      if(Math.abs(phi)<5)
         phi=5*signum(phi);
      double phi_2 = phi*PI/180;
      double lambda_2 = (lambda-Lambda_0)*PI/180;
      double E = lambda_2*Math.sin(phi_2);
      double x= cot(phi_2)*Math.sin(E);
      return x*180/PI;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
      if(Math.abs(phi)<5)
         phi=5*signum(phi);
      double phi_2 = phi*PI/180;
      double lambda_2 = (lambda-Lambda_0)*PI/180;
      double E = lambda_2*Math.sin(phi_2);
      double y = (phi-Phi_0)*PI/180+ cot(phi_2)*(1-Math.cos(E));
      return y*180/PI;

   }

   public boolean showSettings(){
     System.out.println("Polyconic.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Polyconic";
   }


   private double cot(double z){
     return Math.cos(z)/Math.sin(z);
   }

   private double signum(double d){
     return d>=0?1:-1;
   }

   private double Lambda_0 = 0;
   private double Phi_0 = 0;          
   private double secure_distance = 1;

}
