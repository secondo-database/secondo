package project;



public class Cylindrical implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
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
     double y =   Math.tan(phi_2);
     return y*180/PI;
   }

   public boolean showSettings(){
     System.out.println("Cylindrical.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Cylindrical";
   }


   private double Lambda_0 = 0;
   private double secure_distance = 10;

}
