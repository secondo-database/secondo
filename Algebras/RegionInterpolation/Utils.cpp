/*
 
see Utils.h for documentation
 
\tableofcontents
 
*/
   
#include "RegionInterpolator.h"

using namespace std;

namespace RegionInterpol
{
/*
 
1 Static Methods

1.1 getArea()

*/   
 double Utils :: getArea(LineWA linelist[], int linelistlength)
{
    long double res = 0.0;        
    for(int i = 0; i < linelistlength; i++)
    {
      int ip = (i + 1) % linelistlength;
      long double llixlliy = linelist[i].getX() * linelist[i].getY();
      long double llixllipy   = linelist[i].getX() * linelist[ip].getY();
        long double llipxlliy = linelist[ip].getX()* linelist[i].getY();
        long double llipxllipy   = linelist[ip].getX()* linelist[ip].getY();
        long double summe1    = llixlliy         + llixllipy;
        long double summe2    = llipxlliy        + llipxllipy;
        res+=(summe1-summe2) / 2.0;        
    }
    return(res);
}
    
double Utils :: getArea(vector<LineWA*> lines)
{     
    double res = 0.0;        
    for(unsigned int i = 0; i < lines.size(); i++)
    {
        int ip = (i + 1) % lines.size();
        res = res + ((lines[i]->getX() - lines[ip]->getX()) * 
           (lines[i]->getY() + lines[ip]->getY())) / 2.0;
    }
    return(res);
}
    
double Utils :: getArea(vector<LineWA> *lines)
{     
    double res = 0.0;        
    for(unsigned int i = 0; i < lines->size(); i++)
    {
        int ip = (i + 1) % lines->size();
        res = res + ((lines->at(i).getX() - lines->at(ip).getX()) * 
           (lines->at(i).getY() + lines->at(ip).getY())) / 2.0;
    }
    return(res);
}
    
double Utils :: getArea(vector<CHLine> lines)
{     
    double res = 0.0;        
    for(unsigned int i = 0; i < lines.size(); i++)
    {
        int ip = (i + 1) % lines.size();
        res = res + ((lines[i].getX() - lines[ip].getX()) * 
           (lines[i].getY() + lines[ip].getY())) / 2.0;
    }
    return(res);
}
/*

1.1 convexHull()

*/    
vector<LineWA*> Utils :: convexHull(LineWA *lt, int ltlength)
{  
    //int minpoint; 
    double miny, minx, tmpx, tmpy;
    vector<LineWA*> unfinishedhull;
    int a, index,i;
    double hyp;
    LineWA *point1, *point2, tmp;
    //minpoint = 0;
    index = 0;
    miny = numeric_limits<int> :: max();
    minx = numeric_limits<int> :: max();        
    for (a = 0; a < ltlength; a++)        
    {                         
        if ((lt[a].getY()<miny)||((lt[a].getY()==miny)&&(lt[a].getX()<minx)))
        {
            miny = lt[a].getY();
            minx = lt[a].getX();
            index = a;
        }
    }      
    // Swaps the minimum point with the first point.
   tmp = lt[0];
   lt[0] = lt[index];
   lt[index] = tmp;        
   // Find the angles to the other points with respect to the first point.
   lt[0].setAngle(-1.0);// To make sure it is the least when the array is sorted
   for (a = 1; a < ltlength; a++)
   {
      tmpx = lt[a].getX() - lt[0].getX();
      tmpy = lt[a].getY() - lt[0].getY();
      hyp = sqrt(tmpx * tmpx + tmpy * tmpy);
      double angle = acos(tmpx / hyp);                   
      lt[a].setAngle(angle);
   }        
   // Sort the points with respect to the angle
    qsort(lt,ltlength,sizeof(LineWA),compareLineWA);
   vector<LineWA> tmpvec;
   tmpvec.push_back(lt[0]);
   for (int i = 1; i < ltlength; i++)
   {
      if(!AlmostEqual(tmpvec[tmpvec.size() - 1].getAngle(), lt[i].getAngle()))
      {
         tmpvec.push_back(lt[i]);
      }
         else
         {        
            tmpx = lt[0].getX() - lt[i].getX();
               tmpy = lt[0].getY() - lt[i].getY();
               double distli = sqrt(tmpx * tmpx + tmpy * tmpy);
               tmpx = lt[0].getX() - tmpvec[tmpvec.size() - 1].getX();
               tmpy = lt[0].getY() - tmpvec[tmpvec.size() - 1].getY();
               double disttmp = sqrt(tmpx * tmpx + tmpy * tmpy);
               if(distli > disttmp)
               {
               tmpvec.pop_back();
               tmpvec.push_back(lt[i]);   
            }                    
      }
   }  
   ltlength = tmpvec.size();
   for (int i = 0; i < ltlength; i++)
   {
      lt[i] = tmpvec[i];
   }

    // Use graham scan to create convex hull        
   // The point with the lowest y-coordinate is on the hull        
   unfinishedhull.push_back(&lt[0]);
   // The point with the lowest angle with respect to the x-axis is on the
   // hull
    unfinishedhull.push_back(&lt[1]);
    i = 2;
    int N = ltlength;
    while(i < N)
    {
            
        point1 = unfinishedhull.back();
        unfinishedhull.pop_back();
        point2 = unfinishedhull.back();
        unfinishedhull.push_back(point1);
        if(!point1->equals(point2)&& ((sameSide(point1, point2, &lt[i]) <= 0)||
               HalfSegment(true, Point(true, point1->getX(), point1->getY()), 
               Point(true, point2->getX(), point2->getY()))
               .Contains(Point(true, lt[i].getX(), lt[i].getY()))))
        {      
         unfinishedhull.push_back(&lt[i]);
            i++;
        }
        else
        {            
            unfinishedhull.pop_back();
        }            
    }        
    return(unfinishedhull);
}
/*

1.1 compareLineWA()

*/     
int Utils :: compareLineWA(const void *line1, const void *line2)
{
   LineWA* tmp1 = (LineWA*) line1;
   LineWA* tmp2 = (LineWA*) line2;
   return(tmp1->compareTo(tmp2));
}
/*

1.1 sameSide()

*/    
double Utils :: sameSide(LineWA* line1, LineWA* line2, LineWA* p1)
{                 
   long double l1xpy  = line1->getX()  *p1->getY();      
   long double pxl2y  = p1->getX()     *line2->getY();
   long double pxl1y  = p1->getX()     *line1->getY();
   long double l1xl2y    = line1->getX()  *line2->getY();      
   long double l2xpy  = line2->getX()  *p1->getY();
   long double l2xl1y    = line2->getX()  *line1->getY();
   long double summe2 = l2xl1y + l1xpy + pxl2y;
   long double summe1 = l2xpy + pxl1y + l1xl2y;       
    return( (double) (summe1 - summe2));
}
/*

1.1 indexOf()

*/            
int Utils :: indexOf(vector<LineWA*> array, LineWA obj)
{
   unsigned int a;        
    for (a=0;a<array.size();a++)
    {
      if (obj.equals(array[a])) return(a);
    }
    return(-1);
}

int Utils :: indexOf(LineWA *array, LineWA obj)
{
    int a;
    int arraylength = (sizeof(array) / sizeof(*(array)));
    for (a=0;a<arraylength;a++)
    {
      if (obj.equals(&array[a])) return(a);
    }
    return(-1);
}
/*

1.1 getAngleRad()

*/         
double Utils :: getAngleRad(double x, double y, double preX, double preY, 
   double folX, double folY)
{   
    double a = sqrt((preX- folX) * (preX- folX) + (preY- folY) * (preY- folY));
    double b = sqrt((preX - x) * (preX - x) + (preY - y) * (preY - y));
    double c = sqrt((folX - x) * (folX - x) + (folY - y) * (folY - y));
    double angle = acos((a * a - b * b - c * c) / (-2 * b * c));
    if(((x - preX) * (folY - preY) - (folX - preX) * (y - preY)) < 0)
        angle = M_PI * 2 * -angle;
    return (angle);
}
/*

1.1 reverseVector()

*/  
void Utils :: reverseVector(vector<LineWA> *vec)
{
   for(unsigned int i = 0; i < vec->size() / 2; i++)
   {
      LineWA tmp = vec->at(i);
      (*vec)[i] = (*vec)[vec->size() - 1 - i];
      (*vec)[vec->size() - 1 - i] = tmp;
   }
}
/*

1.1 getHausdorfDistance()

*/  
double Utils :: getHausdorfDistance(vector<CHLine> obj1, vector<CHLine> obj2)
{
   if(obj2.size() == 0)
      return(0);
    double res = 0;
    for(unsigned int i = 0; i < obj1.size(); i++)
    {
        res = max(res, getSingleHausdorffDistance(&obj1[i], obj2));
    }
    return(res);
}
/*

1.1 getDiameter()

*/
double Utils :: getDiameter(vector<LineWA*> Poly)
{     
    vector<vector<LineWA*> >tmp;
    tmp.push_back(Poly);       
    return(getMaxDistance(tmp));
}

double Utils::getDiameter(vector<CHLine> Poly)
{
   vector<LineWA*> tmp2;
   for(unsigned int i = 0; i < Poly.size(); i++)
   {
      tmp2.push_back( (LineWA*) &Poly[i]);
   }
    vector<vector<LineWA*> >tmp;        
    tmp.push_back(tmp2);
    return(getMaxDistance(tmp));
}
/*

1.1 getMaxDistance()

*/
double Utils :: getMaxDistance(vector<vector<LineWA*> > Polys)
{ 
    int count = 0;
    for(unsigned int i = 0; i < Polys.size(); i++)
    {
        count += Polys[i].size();
    }
    LineWA tmp[count];
    int counter = 0;
    for(unsigned int i = 0; i < Polys.size(); i++)
    {
        for(unsigned int j = 0 ; j < Polys[i].size(); j++)
        {
            tmp[counter++] = Polys[i][j];
        }
    }
    vector<LineWA*> conTmp = Utils :: convexHull(tmp, count);
    double tmpdist = 0;
    int pos = conTmp.size() / 2;
    for(unsigned int i = 0; i < conTmp.size(); i++)
    {
        pos = getLongestDistFromPoint(conTmp[i], conTmp, pos);
        double distxy = Utils :: getSquareDistance(conTmp[i], conTmp[pos]);
        if(distxy > tmpdist)
        {
            tmpdist = distxy;
        }
    }    
    return(sqrt(tmpdist));
}
/*

1.1 convertCHLine2LineWA()

*/
vector<LineWA*> Utils :: convertCHLine2LineWA(vector<CHLine> vec)
{
      vector<LineWA*> res;
   for(unsigned int i = 0; i < vec.size(); i++)
   {
      res.push_back( (LineWA*) &vec[i]);        
   }
   return(res);
}
/*

1.1 convertLineWA2CHLine()

*/
vector<CHLine> *Utils :: convertLineWA2CHLine(vector<LineWA> *vec)
{
   vector<CHLine> *res = new vector<CHLine>;
   for(unsigned int i = 0; i < vec->size(); i++)
   {
      res->push_back(CHLine(&vec->at(i)));
   }
   return(res);
}
/*

1.1 getSquareDistance()

*/
double Utils :: getSquareDistance(LineWA *p1, LineWA *p2)
{
   long double x1s  = p1->getX() * p1->getX();
   long double x2s  = p2->getX() * p2->getX();
   long double y1s  = p1->getY() * p1->getY();
   long double y2s  = p2->getY() * p2->getY();
   long double x1x2 = p1->getX() * p2->getX();
   long double y1y2 = p1->getY() * p2->getY();
   long double tmp  = x1x2 + y1y2;
   long double res  = x1s + x2s + y1s + y2s - 2 * tmp;
   return((double) res);      
}
/*

1.1 computeLineAngles()

*/
void Utils :: computeLineAngles(vector<LineWA> *lines)
{
    int  listlength, nextpoint, a;
    long double lengthhyp, angle, lengthx, lengthy, sum1, sum2;
    listlength = lines->size()-1;
    for (a = 0; a < listlength; a++)
    {
        nextpoint = a + 1;
        sum1 = lines->at(nextpoint).getX() * lines->at(nextpoint).getX()+
            lines->at(a).getX() * lines->at(a).getX()+
            lines->at(nextpoint).getY() * lines->at(nextpoint).getY()+
            lines->at(a).getY() * lines->at(a).getY();
        sum2 = (lines->at(nextpoint).getX() * lines->at(a).getX()+
            lines->at(nextpoint).getY() * lines->at(a).getY())*2;        
        lengthx = lines->at(nextpoint).getX() - lines->at(a).getX();
        lengthy = lines->at(nextpoint).getY() - lines->at(a).getY();
        lengthhyp = sqrt(sum1 - sum2);
        angle = acos(lengthx / lengthhyp);
        if (lengthy < 0) angle = (M_PI * 2) - angle;
        lines->at(a).setAngle(angle);
    }
    a = listlength;
    nextpoint = 0;
      sum1 = lines->at(nextpoint).getX() * lines->at(nextpoint).getX()+
            lines->at(a).getX() * lines->at(a).getX()+
            lines->at(nextpoint).getY() * lines->at(nextpoint).getY()+
            lines->at(a).getY() * lines->at(a).getY();
        sum2 = (lines->at(nextpoint).getX() * lines->at(a).getX()+
            lines->at(nextpoint).getY() * lines->at(a).getY())*2;
    lengthx = lines->at(nextpoint).getX() - lines->at(a).getX();
    lengthy = lines->at(nextpoint).getY() - lines->at(a).getY();    
    lengthhyp = sqrt(sum1 - sum2);        
    angle = acos(lengthx / lengthhyp);
    if (lengthy < 0) angle = (M_PI*2)-angle;       
    lines->at(a).setAngle(angle);
}
/*

1.1 getOverlap()

*/
double Utils :: getOverlap(vector<CHLine> *l1, vector<CHLine> *l2)
{
#ifdef USE_OVERLAP
   Region *r1 = Utils :: convert2Region(l1);
   Region *r2 = Utils :: convert2Region(l2);    
   Region* res = new Region(0);
    MakeOp mo;
    res = mo.Intersection(r1, r2);
    return(res->Area());
#else
    return(0);
#endif
}
/*

1.1 convert2Region()

*/
Region* Utils :: convert2Region(vector<CHLine> *l1)
{
   Region *res = new Region( (l1->size()) * 2);
   res->Clear();
   res->StartBulkLoad();
   for(unsigned int i = 0; i < l1->size(); i++)
   {
      AttrType attr = AttrType();
      attr.faceno = 0;
      attr.cycleno = 0;
      attr.edgeno = i;         
      Point p1 = Point(true, l1->at(i).getX(), l1->at(i).getY());
      Point p2 = Point(true, l1->at((i + 1) % l1->size()).getX(), 
         l1->at((i + 1) % l1->size()).getY());
      if(p1.GetX() > p2.GetX())
      {
         attr.insideAbove=true;
      }
      else
      {
         if(p1.GetX() == p1.GetX() && p1.GetY() < p1.GetY())
         {
            attr.insideAbove = true;
         }
         else
         {
            attr.insideAbove = false;
         }
      }        
      if(!AlmostEqual(p1, p2))      
      {
         HalfSegment hs1 = HalfSegment(true, p1, p2);
         hs1.SetAttr(attr);      
         (*res)+=hs1;         
         hs1.SetLeftDomPoint(false);
         (*res)+=hs1;         
      }  
   }        
   res->EndBulkLoad(true,true,true,true);   
   res->SetDefined(true);
   return(res);
}
/*

1.1 toString()

*/
string Utils :: toString(const int &t)
{
  std :: ostringstream oss; // create a stream
//  oss << t;       // insert value to stream
  return oss.str();   // return as a string
}
/*

1.1 getRectangularDistance()

*/
double Utils::getRectangularDistance(LineWA *lineA,LineWA *lineB,LineWA *point)
{
   long double t1 = lineA->getX()* lineB->getX()+ lineA->getX()* point->getX()+
      lineA->getY() * lineB->getY() + lineA->getY() * point->getY();
   long double t2 = lineB->getX()* point->getX()+ lineA->getX()* lineA->getX()+
      lineB->getY() * point->getY() + lineA->getY() * lineA->getY();
   long double denom1 = lineA->getX()*lineA->getX()+lineB->getX()*lineB->getX()+
      lineA->getY() * lineA->getY() + lineB->getY() * lineB->getY();
   long double denom2 = lineA->getX() * lineB->getX() + 
      lineA->getY() * lineB->getY();
   long double denom = denom1 - 2 * denom2;
   long double t = -1 * (t1 - t2) / denom;
    if(denom == 0)
    {
      return(numeric_limits<double> :: quiet_NaN());
    }    
    if(t < 0 || t > 1)
    {
        return(numeric_limits<double> :: quiet_NaN());
    }
    long double xfac = lineA->getX() + lineB->getX() * t - 
       (lineA->getX() * t + point->getX());
    long double yfac = lineA->getY() + lineB->getY() * t - 
       (lineA->getY() * t + point->getY());    
    double res = sqrt(xfac * xfac + yfac * yfac);    
    if(Utils :: sameSide(lineA, lineB, point) > 0)
    {
        res = res * -1;
    }
    return res;
}
/*

1.1 getIntersections()

*/
vector<LineWA*> *Utils :: getIntersections(LineWA *lineA, LineWA *lineB, 
   vector<CHLine> *poly)
{
    vector<LineWA*> *intersections = new vector<LineWA*>;
    for(unsigned int i = 0; i < poly->size(); i++)
    {
        LineWA *inter = Utils :: getIntersection(lineA, lineB, 
           (LineWA*) &poly->at(i), (LineWA*)&poly->at((i + 1) % poly->size()));
        if(inter != NULL)
            intersections->push_back(inter);
    }
    return(intersections);
}
/*

1.1 getIntersection()

*/
LineWA *Utils :: getIntersection(LineWA *line1A, LineWA *line1B, 
   LineWA *line2A, LineWA *line2B)
{
    double minx1, maxx1, miny1, maxy1;
    if(line1A->getX() > line1B->getX())
    {
        minx1 = line1B->getX();
        maxx1 = line1A->getX();
    }
    else
    {
        minx1 = line1A->getX();
        maxx1 = line1B->getX();
    }
    if(line1A->getY() > line1B->getY())
    {
        miny1 = line1B->getY();
        maxy1 = line1A->getY();
    }
    else
    {
        miny1 = line1A->getY();
        maxy1 = line1B->getY();
    }
    if(line2A->getX() < minx1 && line2B->getX() < minx1)
        return(NULL);
    if(line2A->getX() > maxx1 && line2B->getX() > maxx1)
        return(NULL);
    if(line2A->getY() < miny1 && line2B->getY()<miny1)
        return(NULL);
    if(line2A->getY() > maxy1 && line2B->getY() > maxy1)
        return(NULL);
    LineWA *res;
    double xlk = line1B->getX() - line1A->getX();
    double ylk = line1B->getY() - line1A->getY();
    double xnm = line2B->getX() - line2A->getX();
    double ynm = line2B->getY() - line2A->getY();
    double xmk = line2A->getX() - line1A->getX();
    double ymk = line2A->getY() - line1A->getY();
    double det = xnm * ylk - ynm * xlk;
    if(det == 0)
    {
        return(NULL);
    }
    else
    {
        double detinv = 1.0 / det;
        double s = (xnm * ymk - ynm * xmk) * detinv;
        double t = (xlk * ymk - ylk * xmk) * detinv;
        if(s < 0 || s > 1 || t < 0 || t > 1)
        {
            return(NULL);
        }
        else
        {
            double x = line1A->getX() + xlk * s;
            double y = line1A->getY() + ylk * s;
            res=new LineWA(x, y);
        }
    }
    return(res);
}
/*

1.1 joinLinelists()

*/
vector<LineWA> *Utils :: joinLinelists(vector<LineWA> *first, 
   vector<LineWA> *second)
{
    try
    {
      Region cycle1 = *Utils :: convert2Region(
         Utils :: convertLineWA2CHLine(first));
      Region cycle2 = *Utils :: convert2Region(
         Utils :: convertLineWA2CHLine(second));
        double area = Utils :: getArea(first);
        if(area < 0)
        {         
            vector<LineWA> *tmplistrev = new vector<LineWA>(first->size());
            for (unsigned int i = 0; i < first->size(); i++)
            {
                tmplistrev->at(first->size() - i - 1) = first->at(i);
            }
            first = tmplistrev;
        }
        area = Utils :: getArea(second);
        if(area > 0)
        {
            vector<LineWA> *tmplistrev = new vector<LineWA>(second->size());
            for (unsigned int i = 0; i < second->size(); i++)
            {
                tmplistrev->at(second->size() - i - 1) = second->at(i);
            }
            second = tmplistrev;
        }
        
        vector<LineWA> *resV = new vector<LineWA>;
        unsigned int startIndex = 0;
        while(cycle2.OnBorder(Point(true, first->at(startIndex).getX(), 
           first->at(startIndex).getY())))
            startIndex++;
        unsigned int i = startIndex;
        while(!Utils :: PointsOnLine(second, &first->at(i), &first->at(i + 1)))
        {
            resV->push_back(first->at(i));
            i++;
        }
        resV->push_back(first->at(i));
        i++;
        LineWA *inter = Utils :: getClosestBoundaryPoint(&first->at(i-1), 
           &first->at(i), second);
        int j = 0;
        while(!second->at(j).equals(inter))
            j++;
        resV->push_back(second->at(j));
        j=(j + 1) % second->size();        
        while(!cycle1.OnBorder(
           Point(true, second->at(j).getX(), second->at(j).getY())))
        {
            resV->push_back(second->at(j));
            j=(j + 1) % second->size();
        }
        resV->push_back(second->at(j));
        while(cycle2.OnBorder(
           Point(true, first->at(i).getX(), first->at(i).getY())))
        {
            i++;
        }
        for(; i<first->size(); i++)
        {
            resV->push_back(first->at(i));
        }
        for(i = 0; i < startIndex; i++)
        {
            resV->push_back(first->at(i));
        }
        return(resV);
    }
    catch(...)
    {
        return(first);
    }
}
/*

1.1 getClosestBoundaryPoint()

*/
LineWA *Utils :: getClosestBoundaryPoint(LineWA *lineA, LineWA *lineB, 
   vector<LineWA> *poly)
{
   vector<LineWA> intersections;
    for(unsigned int i = 0; i < poly->size(); i++)
    {
        if(HalfSegment(true, Point(true, lineA->getX(), lineA->getY()), 
           Point(true, lineB->getX(), lineB->getY())).
         Contains(Point(true, poly->at(i).getX(), poly->at(i).getY())))
        {
            intersections.push_back(poly->at(i));
        }
    }
    if(intersections.size() == 0)
        return(NULL);
    if(intersections.size() == 1)
        return(new LineWA(intersections[0]));
    double minSquareDist = numeric_limits<double> :: max();
    unsigned int minIndex = 0;
    for(unsigned int i = 0; i < intersections.size(); i++)
    {        
        double squareDist = (intersections[i].getX() - lineA->getX()) * 
           (intersections[i].getX() - lineA->getX()) + 
           (intersections[i].getY() - lineA->getY()) * 
           (intersections[i].getY() - lineA->getY());
        if(squareDist < minSquareDist)
        {
            minSquareDist = squareDist;
            minIndex = i;
        }
    }
    return(new LineWA(intersections[minIndex]));
}
/*

1.1 PointsOnLine()

*/
bool Utils :: PointsOnLine(vector<LineWA> *points, LineWA *lineA, LineWA *lineB)
{
   bool res = false;
    for(unsigned int i = 0; i < points->size(); i++)
    {
        if(HalfSegment(true, Point(true, lineA->getX(), lineA->getY()), 
             Point(true, lineB->getX(), lineB->getY()))
            .Contains(Point(true, points->at(i).getX(), points->at(i).getY())))
        {
            res = true;
            break;
        }
    }
    return(res);
}
/*

1 Operators

1.1 $<<$

*/   
ostream& operator << (ostream& s, vector<LineWA> vector)
{
   for(unsigned int i = 0; i < vector.size(); i++)
   {
      s << setw(3) << i << ": " << vector[i];
   }  
   s << endl;
   return s;
}
/*
 
1 Private Methods()
 
1.1 getSingleHausdorffDistance()

*/  
double Utils :: getSingleHausdorffDistance(LineWA *obj1, vector<CHLine> obj2)
{
    double res = (obj1->getX() - obj2[0].getX()) * (obj1->getX() - 
       obj2[0].getX()) + (obj1->getY()-obj2[0].getY()) * 
       (obj1->getY() - obj2[0].getY());
    for(unsigned int i = 1; i < obj2.size(); i++)
    {
        res = std :: min(res, (obj1->getX() - obj2[i].getX()) * 
           (obj1->getX() - obj2[i].getX()) + (obj1->getY() - obj2[i].getY()) * 
           (obj1->getY() - obj2[i].getY()));
    }
    return(sqrt(res));
}    
/*

1.1 PointsOnLine()

*/
int Utils :: getLongestDistFromPoint(LineWA *from, vector<LineWA*> list,int pos)
{
    double d  = Utils :: getSquareDistance(from, list[pos]);
    double dp = Utils :: getSquareDistance(from, list[(pos + 1) % list.size()]);
    double dm = Utils :: getSquareDistance(from, list[(pos - 1 + list.size()) % 
       list.size()]);
    if(d >= dp && d >= dm)
    {
        return(pos);
    }
    else
    {
        if(dp > d)
        {
            return(getLongestDistFromPoint(from, list, (pos+1) % list.size()));
        }
        else
        {
            return(getLongestDistFromPoint(from, list, 
               (pos - 1 + list.size()) % list.size()));
        }
    }
}

}
