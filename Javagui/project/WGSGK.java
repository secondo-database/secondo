package project;


public class WGSGK extends ProjectionAdapter{


public String getName(){ return "WGS2GK";}

private static class P3d{
  double x; double y; double z;
}

private static P3d p3d = new P3d();

public boolean showSettings(){return true;}

public boolean project(double l1, double b1, java.awt.geom.Point2D.Double result){
   
  /*

    // Heimeier version , unprecise with jumps

   wgs2pot(l1,b1,result);

   System.out.println("Potsdam : "+ result.x + ", " + result.y);
   double a = result.x*Pi/180;
   double b = result.y*Pi/180;

   BesselBLnachGaussKrueger(a,b,result);

   return true;
  */


  try{
    // labonde - precise with jumps
	  l1=Pi*l1/180;
	  b1=Pi*b1/180;
     
    double a=awgs;
	  double b=bwgs;
	  double eq=eqwgs;
	  double N=a/Math.sqrt(1-eq*Math.sin(b1)*Math.sin(b1));
	  double Xq=(N+h1)*Math.cos(b1)*Math.cos(l1);
	  double Yq=(N+h1)*Math.cos(b1)*Math.sin(l1);
	  double Zq=((1-eq)*N+h1)*Math.sin(b1);

	  HelmertTransformation(Xq,Yq,Zq,p3d);
	  double X = p3d.x;
	  double Y = p3d.y;
	  double Z = p3d.z;

	  a=abes;
	  b=bbes;
	  eq = eqbes;

	  BLRauenberg(X,Y,Z,p3d);
	  double b2 = p3d.x;
	  double l2 = p3d.y;
	  double h2 = p3d.z;
	  BesselBLnachGaussKrueger(b2,l2,result);
    return true;
  }catch(Exception e){
     e.printStackTrace();
     return false;
  }

}





public static double  Pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164;

private static final double awgs = 6378137.0;         // WGS84 Semi-Major Axis = Equatorial Radius in meters
private static final double bwgs = 6356752.314;      // WGS84 Semi-Minor Axis = Polar Radius in meters
private static final double abes = 6377397.155;       // Bessel Semi-Major Axis = Equatorial Radius in meters
private static final double bbes = 6356078.962;       // Bessel Semi-Minor Axis = Polar Radius in meters
private static final double cbes = 111120.6196;       // Bessel latitude to Gauss-Krueger meters
private static final double dx   = -585.7;                // Translation Parameter 1
private static final double dy   = -87.0;                  // Translation Parameter 2
private static final double dz   = -409.2;                // Translation Parameter 3
private static final double rotx = 2.540423689E-6;   // Rotation Parameter 1
private static final double roty = 7.514612057E-7;   // Rotation Parameter 2
private static final double rotz = -1.368144208E-5;  // Rotation Parameter 3
private static final double sc = 0.99999122;           // Scaling Factor
private static double h1 = 0;
// derived constants
private static double eqwgs = (awgs*awgs-bwgs*bwgs)/(awgs*awgs);
private static double eqbes = (abes*abes-bbes*bbes)/(abes*abes);



// the meridian code digit
private double MDC = 2.0;  // standard in Hagen


void HelmertTransformation(double x,double y,double z,P3d p)
{
  p.x=dx+(sc*(1*x+rotz*y-roty*z));
  p.y=dy+(sc*(-rotz*x+1*y+rotx*z));
  p.z=dz+(sc*(roty*x-rotx*y+1*z));
}

void BesselBLnachGaussKrueger(double b,double ll,java.awt.geom.Point2D.Double result)
{
  double bg=180*b/Pi;
  double lng=180*ll/Pi;
//  double l0=3*round((180*ll/Pi)/3);
  double l0 = 3*MDC;
  l0=Pi*l0/180;
  double l=ll-l0;
  double k=Math.cos(b);
  double t=Math.sin(b)/k;
  double eq=eqbes;
  double Vq=1+eq*k*k;
  double v=Math.sqrt(Vq);
  double Ng=abes*abes/(bbes*v);
  double nk=(abes-bbes)/(abes+bbes);
  double X=((Ng*t*k*k*l*l)/2)+((Ng*t*(9*Vq-t*t-4)*k*k*k*k*l*l*l*l)/24);
  double gg=b+(((-3*nk/2)+(9*nk*nk*nk/16))*Math.sin(2*b)+15*nk*nk*Math.sin(4*b)/16-35*nk*nk*nk*Math.sin(6*b)/48);
  double SS=gg*180*cbes/Pi;
  double Ho=(SS+X);
  double Y=Ng*k*l+Ng*(Vq-t*t)*k*k*k*l*l*l/6+Ng*(5-18*t*t+t*t*t*t)*k*k*k*k*k*l*l*l*l*l/120;
  double kk=500000;
  double Pii=Pi;
  //double RVV=round((180*ll/Pii)/3);
  double RVV = MDC; 
  double Re=RVV*1000000+kk+Y;
  result.x = Re;
  result.y = Ho;
}





void BLRauenberg (double x,double y,double z,P3d result)
{
  double f=Pi*50/180;
  double p=z/Math.sqrt(x*x+y*y);
  double f1,f2,ft;
  do
  {
    f1=neuF(f,x,y,p);
    f2=f;
    f=f1;
    ft=180*f1/Pi;
  }
  while(!(Math.abs(f2-f1)<10E-10));
  result.x=f;
  result.y=Math.atan(y/x);
  result.z=Math.sqrt(x*x+y*y)/Math.cos(f1)-abes/Math.sqrt(1-eqbes*Math.sin(f1)*Math.sin(f1));
}

double neuF(double f,double x,double y,double p)
{
  double zw;
  double nnq;
  zw=abes/Math.sqrt(1-eqbes*Math.sin(f)*Math.sin(f));
  nnq=1-eqbes*zw/(Math.sqrt(x*x+y*y)/Math.cos(f));
  return(Math.atan(p/nnq));
}



double round(double src)
{
  double theInteger;
  double theFraction;
  double criterion = 0.5;

  theInteger = (int) src;
  theFraction = src-theInteger;

  if (!(theFraction < criterion))
  {
    theInteger += 1;
  }

  return theInteger;
}

// heimeier vs. labonde
private void  wgs2pot(double bw, double lw, java.awt.geom.Point2D.Double result){
   double a = 6378137.000;
   double fq = fq = 3.35281066e-3;
   double f = fq - 1.003748e-5;
   double dx = -587;
   double dy = -16;
   double dz = -393;

   double e2q = (2*fq-fq*fq);
   double e2 = (2*f-f*f);
   double b1 = bw * (Pi/180);
   double l1 = lw * (Pi/180);

   double nd = a/Math.sqrt(1 - e2q*Math.sin(b1)*Math.sin(b1));
   double xw = nd*Math.cos(b1)*Math.cos(l1);
   double yw = nd*Math.cos(b1)*Math.sin(l1);
   double zw = (1 - e2q)*nd*Math.sin(b1);

   double x = xw + dx;
   double y = yw + dy;
   double z = zw + dz;
   double rb = Math.sqrt(x*x + y*y);
   double b2 = (180/Pi) * Math.atan((z/rb)/(1-e2));  
   double l2=0;
   if(x>0){
      l2 = (180/Pi) * Math.atan(y/x);
   }else if (x<0 && y>0){
      l2 = (180/Pi) * Math.atan(y/x) + 180; 
   }else if(x<0 && y<0){
      l2 = (180/Pi) * Math.atan(y/x) - 180;
   }
   result.x = l2;
   result.y = b2;
  
}





} // close class
