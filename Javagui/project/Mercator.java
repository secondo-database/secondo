package project;



public class Mercator implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
     //     if(lambda>180 || lambda<-180)
       // throw new InvalidInputException("lambda out of range");
     return lambda-Lambda_0;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     if(phi>90 || phi<-90)
        throw new InvalidInputException("phi out of range");
     if( phi<=(-90+secure_distance))
         // throw new InvalidInputException("phi is to near to a pol");
	phi = -90+secure_distance;
     if( phi>=(90-secure_distance))
        phi = 90-secure_distance;

     double phi_2 = phi*PI/180;
     double y =   Math.log(Math.tan(PI/4 + phi_2/2))/LOG_E;
     return y*180/PI;
   }

   public boolean showSettings(){
     System.out.println("Mercator.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Mercator";
   }


   private double Lambda_0 = 0;
   private double secure_distance = 1;

}
