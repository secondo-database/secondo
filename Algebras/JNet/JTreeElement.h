/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, July Simone Jandt

1 Defines and includes

*/

#ifndef JTREEELEMENT_H
#define JTREEELEMENT_H

#include "RouteLocation.h"
#include "VisitedJunction.h"
#include "JPQEntry.h"

namespace jnetwork {

/*
1 class JTreeElement

Extends an datatype TreeElem by two int values indexing the right and left son
of the TreeElem in an Tree.

*/

template<class TreeElem>
class JTreeElement {

/*
1.1 public declaration part

*/

public:

/*
1.1.1 Constructors and deconstructor

*/

JTreeElement();
JTreeElement(const TreeElem& elem, const int l = -1, const int r = -1);
JTreeElement(const JTreeElement& other);
~JTreeElement();

/*
1.1.1 Getter and Setter

*/

TreeElem GetValue() const;
int GetLeftSon() const;
int GetRightSon() const;

void SetValue(const TreeElem& in);
void SetLeftSon(const int i);
void SetRightSon(const int i);

/*
1.1.1 Some standard operations

*/

JTreeElement<TreeElem>& operator= (const JTreeElement<TreeElem>& nelem);
ostream& Print(ostream&) const;
int Compare(const JTreeElement<TreeElem>& other) const;
int Compare(const TreeElem& other) const;


/*
1.1 private declaration part

*/

private:

  TreeElem value;  //value of tree element
  int left, right; //point by index in array to left resp. right son

};

/*
1 Definitions of JTreeElement Classes

*/

typedef JTreeElement<RouteLocation> JRLTreeElement;
typedef JTreeElement<JPQEntry> JPQEntryTreeElement;
typedef JTreeElement<VisitedJunction> VisitedJunctionTreeElement;

/*
1 Implementation of class JRLTreeElement

1.1 Constructors and Deconstructors

*/

template<class TreeElem>
JTreeElement<TreeElem>::JTreeElement()
{}

template<class TreeElem>
JTreeElement<TreeElem>::JTreeElement(const TreeElem& elem,
                                     const int l /*= -1*/,
                                     const int r /*= -1*/) :
  value(elem), left(l), right(r)
{}

template<class TreeElem>
JTreeElement<TreeElem>::JTreeElement(const JTreeElement<TreeElem>& other) :
  value(other.GetValue()), left(other.GetLeftSon()), right(other.GetRightSon())
{}

template<class TreeElem>
JTreeElement<TreeElem>::~JTreeElement()
{}

/*
1.1 Getter and Setter

*/

template<class TreeElem>
TreeElem JTreeElement<TreeElem>::GetValue() const
{
 return value;
}

template<class TreeElem>
int JTreeElement<TreeElem>::GetLeftSon() const
{
 return left;
}

template<class TreeElem>
int JTreeElement<TreeElem>::GetRightSon() const
{
 return right;
}

template<class TreeElem>
void JTreeElement<TreeElem>::SetValue(const TreeElem& in)
{
 value = in;
}

template<class TreeElem>
void JTreeElement<TreeElem>::SetLeftSon(const int i)
{
 left = i;
}

template<class TreeElem>
void JTreeElement<TreeElem>::SetRightSon(const int i)
{
 right = i;
}

/*
1.1 Some standard operations

*/

template<class TreeElem>
JTreeElement<TreeElem>& JTreeElement<TreeElem>::operator= (
                                          const JTreeElement<TreeElem>& telem)
{
 value = telem.GetValue();
 left = telem.GetLeftSon();
 right = telem.GetRightSon();
 return *this;
}

template<class TreeElem>
ostream& JTreeElement<TreeElem>::Print(ostream& os) const
{
 os << "JTreeElement: " << value
    << ", left son: " << left
    << ", right son: " << right << endl;
 return os;
}

template<class TreeElem>
int JTreeElement<TreeElem>::Compare(const JTreeElement< TreeElem >& other) const
{
  return value.Compare(other.GetValue());
}

template<class TreeElem>
int JTreeElement<TreeElem>::Compare(const TreeElem& other) const
{
  return value.Compare(other);
}

} // end of namespace jnetwork
/*
1 Overwrite output operator

*/

using namespace jnetwork;

template <class TreeElem>
ostream& operator<< (ostream& os, const jnetwork::JTreeElement<TreeElem>& elem)
{
  elem.Print(os);
  return os;
}



#endif //JTREEELEMENT_H