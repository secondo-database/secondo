package project;



public class Miller_Cylindrical implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
     //     if(lambda>180 || lambda<-180)
       // throw new InvalidInputException("lambda out of range");
     return lambda-Lambda_0;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     if(phi>90 || phi<-90)
        throw new InvalidInputException("phi out of range");
     
     double phi_2 = phi*PI/180;
     double y =  5/4* Math.log(Math.tan(PI/4 + 2*(phi_2/5)))/LOG_E;

     return y*180/PI;
   }

   public boolean showSettings(){
     System.out.println("Miller_Cylindrical.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Miller_Cylindrical";
   }


   private double Lambda_0 = 0;
   private double secure_distance = 1;

}
