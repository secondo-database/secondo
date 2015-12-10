/*

1 Lines.h

*/

#ifndef LINEWA_H_
#define LINEWA_H_


namespace RegionInterpol
{

/*
 
1.1 class LineWA

This class repesents a point in two dimensions and an angle. It is used to store a 
polygone as a list of LineWAs. 

~just a forward declaration~

*/ 
   class CHLine;
   
   class LineWA
   {
       public: 
/*
  
1.1.1 Constructors and Destructor

empty constructor

*/    
         LineWA();
/*
 constructor that sets $x$ and $y$
 
*/       
         LineWA( const double xp,const double yp);
/*
 * 
constructor that sets x y and an angle a

*/           
         LineWA(double xp, double yp, double a);
/*
 
A copy constructor

*/       
         LineWA(LineWA* original);
/*
 
A constructor to convert a CHLine in a LineWA

*/       
         LineWA(CHLine* line);
/*
  
The destructor

*/       
         ~LineWA();
/*
         
1.1.1 Get functions

*/
         double getX();
         double getY();
         double getAngle();
/*
  
1.1.1 Set functions
 
*/
         void setX(double x);
         void setY(double y);
         void setAngle(double angle);
/*
           
1.1.1 Public Methods

$compareTo$ compares to LineWA according to their angle 

*/         
         int compareTo(LineWA* line);
         bool equals(LineWA* line);
/*
 
1.1.1 Operators

*/
//         virtual void operator = (LineWA &arg);
         friend std::ostream & operator <<(std::ostream & os,
                                           const LineWA line);   
         friend bool operator< (const LineWA &l2, const LineWA &l1);
      private:
/*
 
1.1.1 Attributes

*/    
         double x;
         double y;
         double angle;
   };
   
/*

1.1 class LineDist

This class repesents a point in two dimensions and a distance. 

*/ 
   
   class LineDist
   {
       public: 
/*
 
1.1.1 Constructors and Destructor

empty constructor

*/     
         LineDist();
/*
 
 constructor that sets $x$ and $y$
 
*/ 
         LineDist(double x,double y);
/*
 
 constructor that sets $x$, $y$ and a $distance$
 
*/             
         LineDist(LineWA* p,double distance);
/* 

the destructor

*/             
         ~LineDist();
/*
           
1.1.1 Get functions

*/       
         double getX();
         double getY();
         double getDistance();
/*
 
1.1.1 Set functions
 
*/       
         void setX(double x);
         void setY(double y);
         void setDistance(double angle);
/*
           
1.1.1 Public Methods

compares two LineDists according to their distance

*/          
         int compareTo(LineDist* line);
/*
 
1.1.1 Operators

*/                
         friend std::ostream & operator <<(std::ostream & os,
                                           const LineDist line);
         friend bool operator< (const LineDist &l2, const LineDist &l1);
      private:
/*
 
1.1.1 Attributes

*/ 
           double x;
           double y;
           double distance;                   
   };


/*
 
1.1 class CHLine

This class represents a point as a part of a $ConvexHullTree$. 
Each CHLine can store a $ConvexHullTreeNode$ as child.

~just a forward declaration~
 
*/
   
   class ConvexHullTreeNode; 
   
   class CHLine:public LineWA
   {
      public:
/*
 
1.1.1 Constructors and Destructor

empty constructor

*/             
         CHLine();
/*
 
 constructor that sets $x$ and $y$
 
*/          
         CHLine(double x,double y);
/*
 
 constructor converts a $LineWA$ in a $CHLine$
 
*/          
         CHLine(LineWA* line);
/*
           
1.1.1 Get functions

*/        
         ConvexHullTreeNode* getChild();
/*
 
1.1.1 Set functions
 
*/    
         void setChild(ConvexHullTreeNode* child);
//         virtual void operator = (CHLine &arg);
         
/*
 
1.1.1 Operators

*/               
         friend std::ostream & operator <<(std::ostream & os,const CHLine line);
      private:
/*
 
1.1.1 Attributes

*/    
         ConvexHullTreeNode* child;
   };
   
/*

1.1 class PointWNL

this class represents a point in three dimensions

*/ 
   
   class PointWNL:public LineWA
   {
      public:
/*
 
1.1.1 Constructors and Destructor

empty constructor

*/       
         PointWNL();
/*
 
 constructor that sets $x$, $y$ and a timestamp $t$
 
*/          
         PointWNL(double x,double y, int t);
/*
 
 constructor that adds a time to a $LineWA$
 
*/          
         PointWNL(LineWA *line, int t);
/*
 
the destructor
 
*/                
         ~PointWNL();
/*
           
1.1.1 Get functions

*/          
         int getT();
/*
           
1.1.1 Public Methods

*/       
         bool equals(PointWNL *other);
/*
 
1.1.1 Operators

*/       
         friend std::ostream & operator <<(std::ostream & os,
                                           const PointWNL line);
      private:
/*
 
1.1.1 Attributes

*/    
         int t;
   };
}

#endif
/*

\pagebreak

*/
  

