package fuzzyobjects.basic;

public class Basic{


/**
 * computes the BasicPoint nearest to (x,y) in R2
 */

public static BasicPoint getNearestBasicPoint(int x,int y) {

 int ltx = (x / a) * a;    // lefttop_x
 int lty = (y / b) * b;    // lefttop_y

 if(x<0)
    ltx=ltx-a;
 if(y<0)
    lty=lty-b;

 int minx = ltx;   // the current nearest point
 int miny = lty;
 double mindist = (x-ltx)*(x-ltx) + (y-lty)*(y-lty);  // quad distance

 int curx = ltx;     // the current point
 int cury = lty+b;
 double curmindist =  (x-curx)*(x-curx) + (y-cury)*(y-cury);
 if (curmindist<mindist) {
     minx = curx;
     miny = cury;
     mindist = curmindist;
 }
 // next point
 cury = lty;
 curx = ltx+a;
 curmindist =  (x-curx)*(x-curx) + (y-cury)*(y-cury);
 if (curmindist<mindist) {
     minx = curx;
     miny = cury;
     mindist = curmindist;
 }
 // next point
 curx = ltx+a/2;
 cury = lty+b/2;
 curmindist =  (x-curx)*(x-curx) + (y-cury)*(y-cury);
 if (curmindist<mindist) {
     minx = curx;
     miny = cury;
     mindist = curmindist;
 }
 // last Point
 curx = ltx+a;
 cury = lty+b;
 curmindist =  (x-curx)*(x-curx) + (y-cury)*(y-cury);
 if (curmindist<mindist) {
     minx = curx;
     miny = cury;
     mindist = curmindist;
 }
 
 return new BasicPoint( minx, miny);

}


/**
 * computes the BasicTriangle nearest to (x,y)
 */

public static BasicTriangle getNearestBasicTriangle(int x,int y) {

int ltx = (x/a)*a;
int lty = (y/b)*b;

if(x<0)
  ltx=ltx-a;
if(y<0)
  lty=lty-b;

/*   (ltx,lty)
         ####################################### (ltx+a,lty)
         # #                                 ###
         #   ##          TOP                ## #
         #     ##                         ##   #
         #       ##                     ##     #
         #         ##                 ##       #
         #           ##             ##         #
         #             ##         ##           #
         #               ##     ##             #
         #                 ## ##               #
         #   LEFT           ##      RIGHT      #
         #                ##  ##               #
         #              ##      ##             #
         #            ##          ##           #
         #          ##              ##         #
         #        ##                  ##       #
         #      ##        BUTTOM        ##     #
         #    ##                          ##   #
         #  ##                              ## #
         ####################################### (ltx+a,lty+b)
    (ltx,lty+b)
*/

x = x - ltx;
y = y - lty;   // move the rectangle to (0,0)

// over the Line (ltx,lty+b) -> (ltx+a,lty) ??
boolean lt = (y*a <= b*(a-x));
// under the other diagonale
boolean lb = (y*a) >= b*x;

BasicPoint P1,P2,P3;

P1 = new BasicPoint(ltx+a/2,lty+b/2); // have all triangles

if (lt) {
   P2 = new BasicPoint(ltx,lty);
   if (lb)
      P3 = new BasicPoint(ltx,lty+b);
   else
      P3 = new BasicPoint(ltx+a,lty);
}
else{    // !lt
    P2 = new BasicPoint(ltx+a,lty+b);
    if(lb) 
       P3 = new BasicPoint(ltx,lty+b);
    else
       P3 = new BasicPoint(ltx+a,lty);
}
return new BasicTriangle(P1,P2,P3);
}


/**
 * computes the BasicSegment nearest to (x,y)
 */

public static BasicSegment getNearestBasicSegment(int x, int y) {

int ltx = (x/a)*a;  // left top of "rectangle"
int lty = (y/b)*b;

if(x<0)
  ltx -=a;
if(y<0)
  lty -=b;

x = x-ltx;          // move Rectangle to (0,0)
y = y-lty;

// compute quadratic distances to vertical and horizontal lines
double [] Ldist = new double[5];
Ldist[0] = y*y;
Ldist[1] = x*x;
Ldist[2] = (b-y)*(b-y);
Ldist[3] = (a-x)*(a-x);

// which diagonle is a candidate ?
boolean left = x<a/2;
boolean top  = y<b/2;

// compute quadratic len of a diagonale (all equal)
double qdiaglen =  a*a/4 + b*b/4;     // (a/2)^2 + (b/2)^2

// compute quadratic len of middle of X and (x,y)
double qXmidLen = (x-a/2)*(x-a/2) + (y-b/2)*(y-b/2);

double qLen;  // quadratic Len from Cornerpoint of Rectangle to (x,y)

if(left)
   if (top)
     qLen = x*x+y*y;
   else
     qLen = x*x + (b-y)*(b-y);
else
  if (top)
     qLen = (a-x)*(a-x) + y*y;
  else
     qLen = (a-x)*(a-x) + (b-y)*(b-y);

// compare "Mathematische Begriffe und Formeln; Side 12

double qarea =
   ( 2*qdiaglen*qXmidLen + 2*qdiaglen*qLen + 2*qXmidLen*qLen
     - qdiaglen*qdiaglen - qXmidLen*qXmidLen - qLen*qLen) / 16;

Ldist[4] =  4*qarea/qdiaglen;

int min=0;
double mindist = Ldist[0];

for(int i=1;i<5;i++){           // genuegt von 1
  if (Ldist[i]<mindist){
     min=i;
     mindist=Ldist[i];
  }
}
BasicPoint P1=null;
BasicPoint P2=null;

switch (min){
  case 0 :  { P1 = new BasicPoint(ltx,lty);
              P2 = new BasicPoint(ltx+a,lty);
              break;
            }
  case 1 : { P1 = new BasicPoint(ltx,lty);
             P2 = new BasicPoint(ltx,lty+b);
             break;
           }
  case 2 : { P1 = new BasicPoint(ltx,lty+b);
             P2 = new BasicPoint(ltx+a,lty+b);
             break;
           }
  case 3 : { P1 = new BasicPoint(ltx+a,lty);
             P2 = new BasicPoint(ltx+a,lty+b);
             break;
           }
  case 4 : { P1 = new BasicPoint(ltx+a/2,lty+b/2);
             if (left)
               if (top)
                  P2 = new BasicPoint(ltx,lty);
               else
                  P2 = new BasicPoint(ltx,lty+b);
             else
               if (top)
                  P2 = new BasicPoint(ltx+a,lty);
               else
                  P2 = new BasicPoint(ltx+a,lty+b);
           }
 } // switch

return new BasicSegment(P1,P2);

}



static int a = fuzzyobjects.Params.a;
static int b = fuzzyobjects.Params.b;
}




