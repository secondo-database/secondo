package project;


public interface Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException;

   public double getPrjY(double lambda, double phi) throws InvalidInputException;

   public boolean showSettings();

   public String getName();

   static final double LOG_E = Math.log(Math.E);
   static final double PI = Math.PI;
 }
