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

*/

/*
Tile includes

*/
#include "../Tile/t/tint.h"
#include "../Tile/t/treal.h"

/*
declaration of namespace GISAlgebra

*/
namespace GISAlgebra {

/*
Method GetValues reads the values for 3x3 cells

parameters: a - reference to top left cell \\
            b - reference to top middle cell \\
            c - reference to top right cell \\
            d - reference to middle left cell \\
            e - reference to middle middle cell \\
            f - reference to middle right cell \\
            g - reference to bottom left cell \\
            h - reference to bottom middle cell \\
            i - reference to bottom right cell \\
            row - number of current row \\
            column - number of current column \\
            currentTuple - number of current tuple \\
            s\_in - current tuple \\
            maxX - maximum X in a tuple \\
            maxY - maximum Y in a tuple \\
            factorNext - if vector current and next have different start points \\
            factorlast - if vector current and last have different start points \\
            skipNextRow - if difference between next and current is more 
                          than one tile \\
            skipLastRow - if difference between last and current is more 
                          than one tile \\
            current - current vector \\
            next - next vector \\
            last - last vector \\
            currentSize - size of current vector \\
            nextSize - size of next vector \\
            lastSize - size of last vector \\
return value: -
exceptions: -

*/

template <typename T, typename SourceTypeProperties>
void GetValues(double* a, double* b, double* c, double* d, double* e,
               double* f, double* g, double* h, double* i,
               int row, int column, int currentTuple, T* s_in,
               int maxX, int maxY,
               int factorNext, int factorLast,
               bool skipNextRow, bool skipLastRow,
               vector<Tuple*> current, vector<Tuple*> next,
               vector<Tuple*> last, 
               int currentSize, int nextSize, int lastSize)
{
  Tuple* tuple_help;
  T* s_in_help;

  // left lower corner
  if ((column == 0) && (row == 0))
  {
    if (currentTuple > 0)
    {
      tuple_help = current[currentTuple - 1];

      if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
      {
        s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

        *a = s_in_help->GetValue((int[]){maxX, 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a))
        {*a = *e;}
        *d = s_in_help->GetValue((int[]){maxX, 0});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d))
        {*d = *e;}
      }
      else
      {
        *a = *d = *e;
      }
    }
    else
    {
      *a = *d = *e;
    }

    *b = s_in->GetValue((int[]){column, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b)){*b = *e;}
    *c = s_in->GetValue((int[]){column + 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c)){*c = *e;}
    *f = s_in->GetValue((int[]){column + 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f)){*f = *e;}

    if ((lastSize > 0) && (skipLastRow == false))
    {
      if ((currentTuple + factorLast > 0) &&
          (currentTuple + factorLast - 1 < lastSize))
      {  
        tuple_help = last[currentTuple - 1 + factorLast];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *g = s_in_help->GetValue((int[]){maxX, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g))
          {*g = *e;}
        }
        else
        {
          *g = *e;
        }
      }
      else
      {
        *g = *e;
      }

      if ((currentTuple + factorLast >= 0) &&
          (currentTuple + factorLast < lastSize))
      {
        tuple_help = last[currentTuple + factorLast];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *h = s_in_help->GetValue((int[]){0, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h))
          {*h = *e;}
          *i = s_in_help->GetValue((int[]){1, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i))
          {*i = *e;}
        }
        else
        {
          *h = *i = *e;
        }
      }
      else
      {
        *h = *i = *e;
      }
    }
    else 
    {
      *g = *h = *i = *e;
    }
  }
  // left upper corner
  else if ((column == 0) && (row == maxY))
  {
    if ((nextSize > 0) && (skipNextRow == false))
    {
      if ((currentTuple + factorNext > 0) &&
          (currentTuple + factorNext - 1 < nextSize))
      {  
        tuple_help = next[currentTuple - 1 + factorNext];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){maxX, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a))
          {*a = *e;}
        }
        else
        {
          *a = *e;
        }
      }
      else
      {
        *a = *e;
      }

      if ((currentTuple + factorNext >= 0) &&
          (currentTuple + factorNext < nextSize))
      {
        tuple_help = next[currentTuple + factorNext];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *b = s_in_help->GetValue((int[]){0, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b))
          {*b = *e;}
          *c = s_in_help->GetValue((int[]){1, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c))
          {*c = *e;}
        }
        else 
        {
          *b = *c = *e;
        }
      }
      else 
      {
        *b = *c = *e;
      }
    }
    else 
    {
      *a = *b = *c = *e;
    }

    if (currentTuple > 0)
    {
      tuple_help = current[currentTuple - 1];

      if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
      {
        s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

        *d = s_in_help->GetValue((int[]){maxX, maxY});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d))
        {*d = *e;}
        *g = s_in_help->GetValue((int[]){maxX, maxY - 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g))
        {*g = *e;}
      }
      else
      {
        *d = *g = *e;
      }
    }
    else
    {
      *d = *g = *e;
    }

    *f = s_in->GetValue((int[]){column + 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f)){*f = *e;}
    *h = s_in->GetValue((int[]){column, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h)){*h = *e;}
    *i = s_in->GetValue((int[]){column + 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i)){*i = *e;}
  }
  // right lower corner
  else if ((column == maxX) && (row == 0))
  {
    *a = s_in->GetValue((int[]){column - 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a)){*a = *e;}
    *b = s_in->GetValue((int[]){column, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b)){*b = *e;}
    *d = s_in->GetValue((int[]){column - 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d)){*d = *e;}

    if (currentTuple + 1 < currentSize)
    {
      tuple_help = current[currentTuple + 1];

      if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
      {
        s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

        *c = s_in_help->GetValue((int[]){0, 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c))
        {*c = *e;}
        *f = s_in_help->GetValue((int[]){0, 0});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f))
        {*f = *e;}
      }
      else
      {
        *c = *f = *e;
      }
    }
    else
    {
      *c = *f = *e;
    }

    if ((lastSize > 0) && (skipLastRow == false))
    {
      if ((currentTuple + factorLast >= 0) &&
          (currentTuple + factorLast < lastSize))
      {
        tuple_help = last[currentTuple + factorLast];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *g = s_in_help->GetValue((int[]){maxX - 1, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g))
          {*g = *e;}
          *h = s_in_help->GetValue((int[]){maxX, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h))
          {*h = *e;}
        }
        else
        {
          *g = *h = *e;
        }
      }
      else
      {
        *g = *h = *e;
      }

      if ((currentTuple + factorLast >= 0) &&
          (currentTuple + factorLast + 1 < lastSize))
      {  
        tuple_help = last[currentTuple + 1 + factorLast];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *i = s_in_help->GetValue((int[]){0, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i))
          {*i = *e;}
        }
        else
        {
          *i = *e;
        }
      }
      else
      {
        *i = *e;
      }
    }
    else 
    {
      *g = *h = *i = *e;
    }
  }
  // right upper corner
  else if ((column == maxX) && (row == maxY))
  {
    if ((nextSize > 0) && (skipNextRow == false))
    {
      if ((currentTuple + factorNext >= 0) &&
          (currentTuple + factorNext < nextSize))
      {
        tuple_help = next[currentTuple + factorNext];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){maxX - 1, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a))
          {*a = *e;}
          *b = s_in_help->GetValue((int[]){maxX, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b))
          {*b = *e;}
        }
        else
        {
          *a = *b = *e;
        }
      }
      else
      {
        *a = *b = *e;
      }
      if ((currentTuple + factorNext >= 0) &&
          (currentTuple + factorNext + 1 < nextSize))
      {  
        tuple_help = next[currentTuple + 1 + factorNext];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *c = s_in_help->GetValue((int[]){0, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c))
          {*c = *e;}
        }
        else
        {
          *c = *e;
        }
      }
      else
      {
        *c = *e;
      }
    }
    else 
    {
      *a = *b = *c = *e;
    }

    *d = s_in->GetValue((int[]){column - 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d)){*d = *e;}
    *g = s_in->GetValue((int[]){column - 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g)){*g = *e;}
    *h = s_in->GetValue((int[]){column, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h)){*h = *e;}

    if (currentTuple + 1 < currentSize)
    {
      tuple_help = current[currentTuple + 1];

      if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
      {
        s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

        *f = s_in_help->GetValue((int[]){0, maxY});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f))
        {*f = *e;}
        *i = s_in_help->GetValue((int[]){0, maxY - 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i)) 
        {*i = *e;}
      }
      else
      {
        *f = *i = *e;
      }
    }
    else
    {
      *f = *i = *e;
    }
  }
  // left column
  else if (column == 0)
  {
    *b = s_in->GetValue((int[]){column, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b)){*b = *e;}
    *c = s_in->GetValue((int[]){column + 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c)){*c = *e;}
    *f = s_in->GetValue((int[]){column + 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f)){*f = *e;}
    *h = s_in->GetValue((int[]){column, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h)){*h = *e;}
    *i = s_in->GetValue((int[]){column + 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i)){*i = *e;}

    if (currentTuple > 0)
    {
      tuple_help = current[currentTuple - 1];

      if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
      {
        s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

        *a = s_in_help->GetValue((int[]){maxX, row + 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a))
        {*a = *e;}
        *d = s_in_help->GetValue((int[]){maxX, row});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d))
        {*d = *e;}
        *g = s_in_help->GetValue((int[]){maxX, row - 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g))
        {*g = *e;}
      }
      else
      {
        *a = *d = *g = *e;
      }
    }
    else
    {
      *a = *d = *g = *e;
    }
  }
  // lower row
  else if (row == 0)
  {
    *a = s_in->GetValue((int[]){column - 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a)){*a = *e;}
    *b = s_in->GetValue((int[]){column, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b)){*b = *e;}
    *c = s_in->GetValue((int[]){column + 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c)){*c = *e;}
    *d = s_in->GetValue((int[]){column - 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d)){*d = *e;}
    *f = s_in->GetValue((int[]){column + 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f)){*f = *e;}

    if ((lastSize > 0)  && (skipLastRow == false))
    {
      if ((currentTuple + factorLast >= 0) &&
          (currentTuple + factorLast < lastSize))
      {
        tuple_help = last[currentTuple + factorLast];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *g = s_in_help->GetValue((int[]){column - 1, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g))
          {*g = *e;}
          *h = s_in_help->GetValue((int[]){column, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h))
          {*h = *e;}
          *i = s_in_help->GetValue((int[]){column + 1, maxY});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i))
          {*i = *e;}
        }
        else
        {
          *g = *h = *i = *e;
        }
      }
      else
      {
        *g = *h = *i = *e;
      }
    }
    else
    {
      *g = *h = *i = *e;
    }
  }
  // right column
  else if (column == maxX)
  {
    *a = s_in->GetValue((int[]){column - 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a)){*a = *e;}
    *b = s_in->GetValue((int[]){column, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b)){*b = *e;}
    *d = s_in->GetValue((int[]){column - 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d)){*d = *e;}
    *g = s_in->GetValue((int[]){column - 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g)){*g = *e;}
    *h = s_in->GetValue((int[]){column, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h)){*h = *e;}

    if (currentTuple + 1 < currentSize)
    {
      tuple_help = current[currentTuple + 1];

      if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
      {
        s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

        *c = s_in_help->GetValue((int[]){0, row + 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c))
        {*c = *e;}
        *f = s_in_help->GetValue((int[]){0, row});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f))
        {*f = *e;}
        *i = s_in_help->GetValue((int[]){0, row - 1});
        if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i))
        {*i = *e;}
      }
      else
      {
        *c = *f = *i = *e;
      }
    }
    else
    {
      *c = *f = *i = *e;
    }
  }
  // upper row
  else if (row == maxY)
  {
    *d = s_in->GetValue((int[]){column - 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d)){*d = *e;}
    *f = s_in->GetValue((int[]){column + 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f)){*f = *e;}
    *g = s_in->GetValue((int[]){column - 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g)){*g = *e;}
    *h = s_in->GetValue((int[]){column, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h)){*h = *e;}
    *i = s_in->GetValue((int[]){column + 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i)){*i = *e;}

    if ((nextSize > 0) && (skipNextRow == false))
    {
      if ((currentTuple + factorNext >= 0) &&
          (currentTuple + factorNext < nextSize))
      {
        tuple_help = next[currentTuple + factorNext];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){column - 1, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a))
          {*a = *e;}
          *b = s_in_help->GetValue((int[]){column, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b))
          {*b = *e;}
          *c = s_in_help->GetValue((int[]){column + 1, 0});
          if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c))
          {*c = *e;}
        }
        else
        {
          *a = *b = *c = *e;
        }
      }
      else
      {
        *a = *b = *c = *e;
      }    
    }
    else
    {
      *a = *b = *c = *e;
    }
  }
  // no border cells
  else
  {
    *a = s_in->GetValue((int[]){column - 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*a)){*a = *e;}
    *b = s_in->GetValue((int[]){column, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*b)){*b = *e;}
    *c = s_in->GetValue((int[]){column + 1, row + 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*c)){*c = *e;}
    *d = s_in->GetValue((int[]){column - 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*d)){*d = *e;}
    *f = s_in->GetValue((int[]){column + 1, row});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*f)){*f = *e;}
    *g = s_in->GetValue((int[]){column - 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*g)){*g = *e;}
    *h = s_in->GetValue((int[]){column, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*h)){*h = *e;}
    *i = s_in->GetValue((int[]){column + 1, row - 1});
    if (SourceTypeProperties::TypeProperties::IsUndefinedValue(*i)){*i = *e;}
  }
}

}
