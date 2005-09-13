import sj.lang.ListExpr;

/**
  * This class is the implemtation of an affine transformation in the
  * Euclidian Plane.
  * A such Transformation is represented a 3x3 Matrix. This
  * representation is well known from the computer graphic.
**/
public class ATransform{

/**
  * creates a Transform without any effect
**/
public ATransform(){
 setIdentity();
}

/** sets this Transform to the identity
  *
 **/
public void setIdentity(){
   for(int i=0;i<3;i++)
    for(int j=0;j<3;j++)
       if(i==j)
          Matrix[i][j]=1;
       else
          Matrix[i][j]=0;
}


/**
  *  Transforms the given Point
 **/
public void transformPoint(Point P){
   double x = P.getX();
   double y = P.getY();
   double z = 1.0;
   P.moveTo(x*Matrix[0][0]+y*Matrix[0][1]+z*Matrix[0][2],
            x*Matrix[1][0]+y*Matrix[1][1]+z*Matrix[1][2]);
}

/** Reads the Transformation from itts nested list representation.
  * A Transformation is a list of simple transformations which
  * are concatented in this method.
  * The Representation is: <br>
  * AT = ( simple<sub>1</sub> simple<sub>2</sub> ... simple<sub>n</sub>) \\
  * where each simple<sub>i</sub> is one of the following \\
  * <ul>
  *   <li>  (rotate angle) </li>
  *   <li>  (scale (x y)) </li>
  *   <li>  (translate (x y)) </li>
  * </ul>
  *  angle, x and y are numeric values
 **/
public boolean readFrom(ListExpr LE){
   setIdentity();
   if(LE.atomType()!=LE.NO_ATOM)
      return false;
   while(!LE.isEmpty()){
      if(!addTransform(LE.first()))
         return  false;
      LE = LE.rest();
   }
   return true;
}

/*
 *  adds a simple Transformation given in its nested list representation
 *  to this affine transformation
 */
private boolean addTransform(ListExpr LE){
   if(LE.atomType()!=LE.NO_ATOM)
      return false;
   if(LE.listLength()!=2)
      return false;
   ListExpr TypeList = LE.first();
   ListExpr ValueList = LE.second();
   if(TypeList.atomType()!=LE.SYMBOL_ATOM)
      return false;
   String Type = TypeList.symbolValue();
   boolean res = false;
   if(Type.equals("rotate"))
      res= addRotation(ValueList);
   else if(Type.equals("scale"))
      res = addScale(ValueList);
   else if(Type.equals("translate"))
     res = addTranslation(ValueList);
   return res;
}

/**
  * adds a rotation to this transformation.
  * The angle is given in LE as numeric value.
**/
private boolean addRotation(ListExpr LE){
   Double D = viewer.hoese.LEUtils.readNumeric(LE);
   if(D==null)
      return false;
   rotate(D.doubleValue());
   return true;
}

/**  appends a scale to this transformation.
  *  The scale factors are given in LE.
**/
private boolean addScale(ListExpr LE){
   if(LE.listLength()!=2)
      return false;
   Double Sx = viewer.hoese.LEUtils.readNumeric(LE.first());
   Double Sy = viewer.hoese.LEUtils.readNumeric(LE.second());
   if(Sx==null || Sy==null)
      return false;
   scale(Sx.doubleValue(),Sy.doubleValue());
   return true;
}

/** appends a translation  to this Transformation.
  * The vector is given as nested list.
**/
private boolean addTranslation(ListExpr LE){
    if(LE.listLength()!=2)
      return false;
   Double Sx = viewer.hoese.LEUtils.readNumeric(LE.first());
   Double Sy = viewer.hoese.LEUtils.readNumeric(LE.second());
   if(Sx==null || Sy==null)
      return false;
   translate(Sx.doubleValue(),Sy.doubleValue());
   return true;
}

/**  writes the internal matrix to the error output
  *
**/
private void writeMatrix(){
   System.err.println("");
   for(int i=0;i<3;i++){
      for(int j=0;j<3;j++)
         System.err.print(Matrix[i][j]+"   ");
      System.err.println("");
   }
}

/** appends a scale with the given values to this.
**/
public void scale(double sx, double sy){
  setTmpToZero();
  MatrixTmp[0][0] = sx;
  MatrixTmp[1][1] = sy;
  MatrixTmp[2][2] = 1;
  MulMatrices();
}

/** appends a rotation with the given angle to this
**/
public void rotate(double angle){
  double ca = Math.cos(angle);
  double sa = Math.sin(angle);
  MatrixTmp[0][0] = ca;
  MatrixTmp[0][1] = -sa;
  MatrixTmp[0][2] = 0;
  MatrixTmp[1][0] = sa;
  MatrixTmp[1][1] = ca;
  MatrixTmp[1][2] = 0;
  MatrixTmp[2][0] = 0;
  MatrixTmp[2][1] = 0;
  MatrixTmp[2][2] = 1;
  MulMatrices();
}

/** appends a translation to this
**/
public void translate(double tx, double ty){
   MatrixTmp[0][0] =1;
   MatrixTmp[0][1] =0;
   MatrixTmp[0][2] =tx;
   MatrixTmp[1][0] =0;
   MatrixTmp[1][1] =1;
   MatrixTmp[1][2] =ty;
   MatrixTmp[2][0] =0;
   MatrixTmp[2][1] =0;
   MatrixTmp[2][2] =1;
   MulMatrices();
}


/** sets a internal used matrix to be zero on all positions
**/
private void setTmpToZero(){
   for(int i=0;i<3;i++)
      for(int j=0;j<3;j++)
         MatrixTmp[i][j] = 0;
}


/**
  * computes Matrix * MatrixTmp and stores the result in Matrix
**/
private void MulMatrices(){
  // create a copy of Matrix
  double[][] TMP = new double[3][3];
  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++)
       TMP[i][j] = Matrix[i][j];

  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++){
      double entry = 0;
      for(int z=0;z<3;z++)
         entry = entry + TMP[i][z]*MatrixTmp[z][j];
      Matrix[i][j] = entry;
    }
}
// the representation of this Transformation
private double[][] Matrix = new double[3][3];
// a temporaly matrix to avoid the creation of many instances of this
private double[][] MatrixTmp = new double[3][3];
}
