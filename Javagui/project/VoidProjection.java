package project;



public class VoidProjection implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
      return lambda;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     return phi;
   }

   public boolean showSettings(){
     System.out.println("VoidProjection.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "VoidProjection";
   }

}
