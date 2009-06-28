/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

2009.6 Thomas Behr

*/
#include <iostream>

using namespace std;

double myabs(const double d){
   return  d<0?-d:d;
}

//bool MyAlmostEqual(const double d1, const double d2){
//   return myabs(d1-d2) < 0.000001;
//}

double mymax(const double d1, const double d2){
   return d1<d2?d2:d1;
}

double mymin(const double d1, const double d2){
   return d1>d2?d2:d1;
}

template<class timeType>
class ICNode{
  public:
     // construct a new non-covered node for the give intervall
     ICNode(const timeType& ts1,
            const timeType& te1): ts(ts1),te(te1){
        cover = false;
        left = 0;
        right = 0;
     }

     ~ICNode(){
        if(left) delete left;
        if(right) delete right;
        left = 0;
        right = 0;
      }

     bool IsCovered(){
       return cover;
     }

     // inserts a new interval
     ICNode* insert(const timeType& ts,
                    const timeType& te){
         return insertrec(ts,te);
     }

     ostream& Print(ostream& o){
       o << "<" <<ts << ", " << te << ", " << cover;
       if(left){
          o << "-";
          left->Print(o);
          o << "-";
          right->Print(o);
       }
       o << ">";
       return o;
     }


  private:
    timeType ts;
    timeType te;
    bool cover;
    ICNode<timeType>* left;
    ICNode<timeType>* right;

    // constructs a new node from the sons
    ICNode(ICNode<timeType>* left, ICNode<timeType>* right){
        ts = left->ts;
        te = right->te;
        cover = false;
        this->left = left;
        this->right = right;
    }

    ICNode<timeType>*  insertrec(const timeType& ts1,
                              const timeType& te1){
       if( cover ){ // already covered
          return this;
       }
       if(te1<=ts){
         return this;
       }
       if(ts1>=te){
         return this;
       }
       // restruct the interval to insert to the stored interval
       timeType ts2 = mymax(ts1,ts);
       timeType te2 = mymin(te1,te);

       if(AlmostEqual(ts2,te2)){
          return this;
       }

       if(AlmostEqual(ts2,ts) && AlmostEqual(te2,te)){
          cover = true;
          if(left) delete left;
          if(right) delete right;
          left = 0;
          right = 0;
          return this;
       }

       if(left==0 ) { // leaf
        if(AlmostEqual(ts2,ts)){
           ICNode* n1 = new ICNode(ts2,te2);
           n1->cover = true;
           ICNode* n2 = new ICNode(te2,te);
           left = n1;
           right = n2;
           return this;
        } else if(AlmostEqual(te2,te)){
           ICNode* n1 = new ICNode(ts,ts2);
           ICNode* n2 = new ICNode(ts2,te2);
           n2->cover = true;
           left = n1;
           right = n2;
           return this;
        } else { // interval in the middle of the covered one
           ICNode* n1 = new ICNode(ts,ts2);
           ICNode* n2 = new ICNode(ts2,te2);
           n2->cover = true;
           ICNode* n3 = new ICNode(te2,te);
           ICNode* n4 = new ICNode(n1,n2);
           left = n4;
           right = n3;
           return this;
        }
     } else {// inner node
       left = left->insert(ts2,te2);
       right = right->insert(ts2,te2);
       if(left->cover && right->cover){
         cover = true;
         delete left;
         delete right;
         left = 0;
         right = 0;
       }
       return this;
     }
     cerr << "This point should never be reached" << endl;
  }
};

template<class timeType>
class CIC{
  public:
   CIC(const timeType& ts, const timeType& te){
     root = new ICNode<timeType>(ts,te);
   }

   ~CIC(){ delete root; root = 0;}

   void insert(const timeType& ts, const timeType& te){
     root = root->insert(ts,te);
   }

   bool IsCovered(){
      return root->IsCovered();
   }

   ostream& Print(ostream& o){
      if(root){
         root->Print(o);
      } else {
         o << "<empty>";
      }
      return o;
   }

   private:
      ICNode<timeType>* root;
};
