/*
This class calculates those values, that are fixed, as soon as its 
constructor is called.

*/
using namespace std;
#include "LATransform.h"; 
/*
This is the constructor. It gets a linear movement (x,y), the 
middle of a 
circle (xm, ym) and an ankle alpha. 
This is possible because another class knows t and therefore does 
already know alpha, x and y. 
Later on, the methods of this class can be called for various points,
without the
already given information. 

*/  
  LATransform::LATransform(double x, double y, double xm, double ym, 
   double alpha)
    {
      //A'=M*A+(D-M*D+W) mit Gesamtgleichung
      //M einfach alpha einsetzen Winkel alpha, M Matrix, zu 
      //füllen in a.. Werte
      //D=(xm, ym), Drehpunkt
      //W=(x,y) Verschiebung
      //in private variablen (schon angelegt)
      a00=cos(alpha);
      a01=-sin(alpha);
      a10=sin(alpha);
      a11=cos(alpha);
      cx=xm-(a00*xm +a01*ym)+x;
      cy=ym-(a10*xm+a11*ym)+y;
      //printf("Hello World. I am a LATransform object.\n");
    }
    
/*
This method calculates the new x value that the given point will get 
after its
movement.

*/
    double LATransform::getImgX(double x, double y){
      double tmp=0;
      tmp=a00*x+a01*y+cx;
      return tmp;
    }
/*
This method calculates the new y value that the given point will 
get after its 
movement.

*/
    double LATransform::getImgY(double x, double y){
      double tmp;
      tmp=a10*x+a11*y+cy;
      return tmp;
    }
/*
usual destructor

*/
    LATransform::~LATransform(){}
;