package viewer.viewer3d.objects;

import sj.lang.ListExpr;

class SingleFPoint{

int x;
int y;
double z;

SingleFPoint(){
  x=0;
  y=0;
  z=1.0;
}



public boolean readFromListExpr(ListExpr LE){
   if (LE==null)
      return false;
   if(LE.listLength()!=3)
      return false;
   ListExpr LE1 = LE.first();
   ListExpr LE2 = LE.second();
   ListExpr LE3 = LE.third(); 
   int x,y;
   float z;     
   if(LE1.isAtom() && LE1.atomType()==ListExpr.INT_ATOM)
      x = LE1.intValue();
   else
      return false;

   if(LE2.isAtom() && LE2.atomType()==ListExpr.INT_ATOM)
      y = LE2.intValue();
    else
      return false;

    if(LE3.isAtom() && ( LE3.atomType()==ListExpr.INT_ATOM | LE3.atomType()==ListExpr.REAL_ATOM))
       if (LE3.atomType()==ListExpr.INT_ATOM)
          z=LE3.intValue();
       else
          z=LE3.realValue(); 
    else
       return false;

    if(z<0 | z>1)
       return false;

    this.x = x;
    this.y = y;
    this.z = z; 
    return true;  
}

}