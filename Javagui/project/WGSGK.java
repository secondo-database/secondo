package project;


import javax.swing.*;

public class WGSGK extends ProjectionAdapter{


public String getName(){ return "Gausss-Krueger";}

private static class P3d{
  double x; double y; double z;
}


public boolean showSettings(){
  gKSettings.setVisible(true);
  return true;

}

public boolean project(double l1, double b1, java.awt.geom.Point2D.Double result){
  if(!useWGS){
     try{
         double a = l1*Pi/180;
         double b = b1*Pi/180;
         BesselBLnachGaussKrueger(b,a,result);
         return true;
     } catch(Exception e){
         e.printStackTrace();
         return false;
     }
  }
   
  try{
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

public boolean isReversible(){ return true; }

public boolean getOrig(double px, double py, java.awt.geom.Point2D.Double result){
  return  gk2geo(px,py,result);
}


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

public void enableWGS(boolean enabled){
   gKSettings.UseWGS.setSelected(enabled);
   useWGS=enabled;
}

public void setMeridian(int m){
    MDC = m;
    gKSettings.MDCField.setText(""+m);
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
// point for shipping results
private static P3d p3d = new P3d();
// the meridian code digit
private double MDC = 2.0;  // standard in Hagena
private boolean useWGS = true; // usw coordinates in wgs ellipsoid
private GKSettings gKSettings = new GKSettings(this);


/** This function computes the geo coordinates from the gauss krueger ones
  * This is the java transformation of a Java script found at:
  * http://calc.gknavigation.de/
  **/
public boolean gk2geo(double GKRight, double GKHeight, java.awt.geom.Point2D.Double result){
   if(GKRight<1000000 || GKHeight<1000000)
       return false;
   double e2 = 0.0067192188;
   double c = 6398786.849;

   double bI = GKHeight/10000855.7646;
   double bII = bI*bI;
   double bf = 325632.08677 * bI * ((((((0.00000562025 * bII + 0.00022976983) 
                                  * bII - 0.00113566119) 
                                  * bII + 0.00424914906) 
                                  * bII - 0.00831729565) 
                                  * bII + 1));
   bf /= 3600*rho;
   double co = Math.cos(bf);
   double g2 = e2 *(co*co);
   double g1 = c/Math.sqrt(1+g2);
   double t = Math.tan(bf);
   double fa = (GKRight - Math.floor(GKRight/1000000)*1000000-500000)/g1;
   double GeoDezRight = ((bf - fa * fa * t * (1 + g2) / 2 + 
                          fa * fa * fa * fa * t * (5 + 3 * t * t + 6 * g2 - 6 * g2 * t * t) / 24) * rho);
   double dl = fa - fa * fa * fa * (1 + 2 * t * t + g2) / 6 + 
               fa * fa * fa * fa * fa * (1 + 28 * t * t + 24 * t * t * t * t) / 120;
   
   double Mer = Math.floor(GKRight/1000000);
   double GeoDezHeight = dl*rho/co+Mer*3;
   if(useWGS){
      return bessel2WGS(GeoDezRight,GeoDezHeight,result);
   }else{
      result.setLocation(GeoDezHeight,GeoDezRight);
      return true;
   }
}

private boolean bessel2WGS(double geoDezRight, double geoDezHeight, java.awt.geom.Point2D.Double result){
    double aBessel = abes;
    double eeBessel = 0.0066743722296294277832;
    double ScaleFactor = 0.00000982;
    double RotXRad = -7.16069806998785E-06;
    double RotYRad = 3.56822869296619E-07;
    double RotZRad = 7.06858347057704E-06;
    double ShiftXMeters = 591.28;
    double ShiftYMeters = 81.35;
    double ShiftZMeters = 396.39;
    double aWGS84 = awgs;
    double eeWGS84 = 0.0066943799;
    geoDezRight = (geoDezRight/180)*PI;
    geoDezHeight = (geoDezHeight/180)*PI;
    double sinRight = Math.sin(geoDezRight);
    double sinRight2 = sinRight*sinRight;
    double n = eeBessel*sinRight2;
    n = 1-n;
    n = Math.sqrt(n);
    n = aBessel/n;
    double cosRight=Math.cos(geoDezRight);
    double cosHeight=Math.cos(geoDezHeight);
    double sinHeight = Math.sin(geoDezHeight);
    double CartesianXMeters = n*cosRight*cosHeight;
    double CartesianYMeters = n*cosRight*sinHeight;
    double CartesianZMeters = n*(1-eeBessel)*sinRight;

    double CartOutputXMeters = (1 + ScaleFactor) * CartesianXMeters + RotZRad * CartesianYMeters - 
                               RotYRad * CartesianZMeters + ShiftXMeters;    
    double CartOutputYMeters = -RotZRad * CartesianXMeters + (1 + ScaleFactor) * CartesianYMeters + 
                               RotXRad * CartesianZMeters + ShiftYMeters;
    double CartOutputZMeters = RotYRad * CartesianXMeters - RotXRad * CartesianYMeters + 
                               (1 + ScaleFactor) * CartesianZMeters + ShiftZMeters;
    
     geoDezHeight = Math.atan(CartOutputYMeters/CartOutputXMeters);
     double Latitude = (CartOutputXMeters*CartOutputXMeters)+(CartOutputYMeters*CartOutputYMeters);
     Latitude = Math.sqrt(Latitude);
     double InitLat = Latitude;
     Latitude = CartOutputZMeters/Latitude;
     Latitude = Math.atan(Latitude);
     double LatitudeIt = 99999999;
     do{
        LatitudeIt = Latitude;
        double sinLat = Math.sin(Latitude);
        n = 1-eeWGS84*sinLat*sinLat;
        n = Math.sqrt(n);
        n = aWGS84/n;
        Latitude = InitLat; // sqrt(CartoutputXMeters^2+CartOutputYMeters^2)
        Latitude = (CartOutputZMeters+eeWGS84*n*Math.sin(LatitudeIt))/Latitude;
        Latitude = Math.atan(Latitude);
     } while(Math.abs(Latitude-LatitudeIt)>=0.000000000000001);

     result.y = (Latitude/PI)*180;
     result.x = (geoDezHeight/PI)*180;
     return true;    
}



private static final double rho = 180/PI;

private static class GKSettings extends javax.swing.JDialog{

  public GKSettings(WGSGK owner){
      super(parent);
      this.owner = owner; 
      setSize(640,480);
      ButtonGroup SourceEllipsoid = new ButtonGroup();     
      SourceEllipsoid.add(UseWGS=new JRadioButton("WGS 84"));
      SourceEllipsoid.add(UseBessel=new JRadioButton("Bessel"));
      MDCField = new JTextField(4);
      MDCField.setText("2");
      JPanel P = new JPanel(new java.awt.GridLayout(3,1)); 
      UseWGS.setSelected(true);
      P.add(UseWGS);
      P.add(UseBessel);
      P.add(MDCField);     
      getContentPane().setLayout(new java.awt.BorderLayout()); 
      JPanel P1 = new JPanel(new java.awt.FlowLayout());
      P1.add(P);
      getContentPane().add(P1,java.awt.BorderLayout.CENTER);
      JPanel P2 = new JPanel(new java.awt.GridLayout(1,2));
      java.awt.event.ActionListener AL = new java.awt.event.ActionListener(){
         public void actionPerformed(java.awt.event.ActionEvent evt){
            Object src = evt.getSource();
            if(okButton.equals(src)){
                 try{
                    GKSettings.this.owner.MDC = Integer.parseInt(MDCField.getText());
                    GKSettings.this.owner.useWGS=UseWGS.isSelected();
                    setVisible(false);
                 }catch(Exception e){
                      
                 }

            }else{
              if(GKSettings.this.owner.useWGS){
                 UseWGS.setSelected(true);
              } else
                 UseBessel.setSelected(true);
              MDCField.setText(""+GKSettings.this.owner.MDC);
              setVisible(false);
            }
         }
      };
      (okButton = new JButton("ok")).addActionListener(AL);
      (cancelButton = new JButton("cancel")).addActionListener(AL);
      P2.add(okButton);
      P2.add(cancelButton);
      getContentPane().add(P2,java.awt.BorderLayout.SOUTH); 
  }

  static javax.swing.JFrame parent=null;
  JRadioButton UseWGS;
  JRadioButton UseBessel;
  JTextField   MDCField;
  WGSGK owner;
  JButton okButton;
  JButton cancelButton;


}


} // close class
