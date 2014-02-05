
package viewer.hoese;


import java.awt.geom.AffineTransform;

public class Auxiliary{

  public static boolean almostEqual(double d1, double d2){
     return Math.abs(d1-d2) < 0.00000001; 
  }


  public static void setTranslation(AffineTransform at, double tx, double ty){
      at.getMatrix(flat); // m00 m10 m01 m11 m02 m12
                          //  0   1   2   3   4   5
      // m02 = translate x
      // m12 = translate y
      flat[4] = tx;
      flat[5] = ty;
      at.setTransform(flat[0], flat[1], flat[2], 
                      flat[3], flat[4], flat[5] );

  }
  

  public static void setScale(AffineTransform at, double sx, double sy){
      at.getMatrix(flat);
      // m00 = scale x
      // m11 = scale y
      flat[0] = sx;
      flat[3] = sy;
      at.setTransform(flat[0], flat[1], flat[2], 
                      flat[3], flat[4], flat[5] );

  }



  public static void scaleWithTranslation(AffineTransform at, double scale){
      at.getMatrix(flat);
      // m00 = scale x
      // m11 = scale y
      flat[0] *= scale;
      flat[3] *=scale;
      flat[4] *=scale;
      flat[5] *=scale;
      at.setTransform(flat[0], flat[1], flat[2], 
                      flat[3], flat[4], flat[5] );
  }


  private static double[] flat = new double[6];

}
